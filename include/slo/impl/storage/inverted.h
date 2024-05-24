#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>

#include <slo/impl/feature.h>
#include <slo/impl/union/recursive.h>
#include <slo/impl/union/tree.h>
#include <slo/util/list.h>

#include "common.h"

namespace slo::impl {
template <auto Idx, typename ValueType>
struct TaggedWrapper {
  using type = ValueType;

  template <typename... Args>
  constexpr explicit TaggedWrapper(Args&&... args) : tag{Idx}
                                                   , storage{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  [[nodiscard]] constexpr ValueType& unwrap() & { return storage; }
  [[nodiscard]] constexpr ValueType const& unwrap() const& { return storage; }
  [[nodiscard]] constexpr ValueType&& unwrap() && { return std::move(storage); }
  [[nodiscard]] constexpr ValueType const&& unwrap() const&& { return std::move(storage); }

  decltype(Idx) tag;
  ValueType storage;
};

/**
 * @brief Storage for an inverted variant. This attempts to reduce padding by hiding the
 *        tag in the alternatives themselves. Outside of constant evaluated context the 
 *        current tag can be retrieved through an inactive member, since all alternatives
 *        share the same common initial sequence.
 *
 *        Due to [expr.const]/5.10 reading the tag through an inactive member is not possible.
 *        To work around that C++26 `std::is_within_lifetime` or clang/gcc's intrinsic
 *        `__builtin_constant_p` can be used to recursively determine which member is active.
 *        The tag can then be read from the active member, which in turn allows visitation of
 *        the variant.
 *
 * @warning Requires C++26 `std::is_within_lifetime` or clang/gcc `__builtin_constant_p` intrinsic
 *          to work in constant evaluated context.
 *
 * @warning [class.mem]/26 mandates standard-layout unions for this technique, consequently it will
 *          only work if all alternatives are standard-layout types.
 * 
 * @tparam Ts... Alternative types. Must be standard layout.
 */
template <typename... Ts>
union InvertedStorage {
  // inverted variant
  static_assert((std::is_standard_layout_v<Ts> && ...), 
                "[class.mem]/26 mandates standard-layout unions for this technique. "
                "This means only standard-layout alternatives can be used.");

  using alternatives                              = util::TypeList<Ts...>;
  using index_type                                = alternatives::index_type;
  constexpr static index_type npos                = static_cast<index_type>(~0U);
  constexpr static bool is_trivially_destructible = (std::is_trivially_destructible_v<Ts> && ...);

  template <std::size_t... Is>
  static constexpr auto generate_union(std::index_sequence<Is...>)
      -> RecursiveUnion<is_trivially_destructible, TaggedWrapper<static_cast<index_type>(Is), Ts>...>;

  template <std::size_t... Is>
    requires(sizeof...(Ts) >= SLO_TREE_THRESHOLD)
  static constexpr auto generate_union(std::index_sequence<Is...>)
      -> TreeUnion<is_trivially_destructible, TaggedWrapper<static_cast<index_type>(Is), Ts>...>;

  using union_type = decltype(generate_union(std::index_sequence_for<Ts...>()));

  TaggedWrapper<npos, error_type> dummy;
  struct Container {
    union_type value;
  } storage;

public:
  template <std::size_t Idx, typename... Args>
  constexpr explicit InvertedStorage(std::in_place_index_t<Idx> idx, Args&&... args)
      : storage{union_type(idx, std::forward<Args>(args)...)} {}

  constexpr InvertedStorage() : dummy() {}
  constexpr InvertedStorage(InvertedStorage const& other)  = default;
  constexpr InvertedStorage(InvertedStorage&& other)       = default;
  InvertedStorage& operator=(InvertedStorage const& other) = default;
  InvertedStorage& operator=(InvertedStorage&& other)      = default;

  constexpr ~InvertedStorage() requires is_trivially_destructible = default;
  constexpr ~InvertedStorage() { reset(); }

  [[nodiscard]] constexpr std::size_t index() const {
    if consteval {
      if (!compat::is_within_lifetime(&dummy.tag)) {
        return storage.value.index();
      }
    }
    return dummy.index();
  }

  constexpr void reset() {
    if (index() != npos) {
      slo::visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
      std::construct_at(&dummy);
    }
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&storage.value, std::in_place_index<Idx>, std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args) {
    emplace<alternatives::template get_index<T>>(std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.value.template get<Idx>();
  }

  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return index() == npos; }
};

}  // namespace slo::impl