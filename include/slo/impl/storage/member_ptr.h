#pragma once
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include <slo/impl/feature.h>
#include <slo/util/list.h>
#include <slo/util/concepts.h>
#include "common.h"

// TODO implement proper feature flags
#if USING(SLO_HIDE_IN_PADDING)
#  define SLO_UNION_ATTR [[no_unique_address]]
#else
#  define SLO_UNION_ATTR
#endif

namespace slo::impl {
namespace detail {
template <auto>
struct MemberPtr;
template <typename C, typename R, R C::* Ptr>
struct MemberPtr<Ptr> {
  using type                    = R;
  using class_type              = C;
  constexpr static R C::* value = Ptr;
};

template <typename... Ptrs>
struct StorageProxy {
  static_assert(util::is_homogeneous<typename Ptrs::class_type...>,
                "All member pointers must refer to subobjects of the same type.");

  using alternatives = util::TypeList<typename Ptrs::type...>;
  using union_type   = util::TypeList<typename Ptrs::class_type...>::template get<0>;
  static_assert(std::is_union_v<union_type>, "All member pointers must refer to subobjects of a union.");

  using index_type                 = alternatives::index_type;
  constexpr static index_type npos = static_cast<index_type>(~0U);

private:
// With SLO_HIDE_IN_PADDING is enabled, the tag _may_ be hidden in padding.
// Unfortunately certain operations (such as move construction)  _may_ clobber
// tail padding, hence we must assume the tag is lost afterwards. This shouldn't
// be a problem since we set the tag after construction anyway.

// It's possibly problematic if a constructor throws - the variant might end up with
// an overwritten tag that suggests it is still valid.

// See https://github.com/llvm/llvm-project/issues/60711
#if USING(SLO_HIDE_IN_PADDING)
  [[no_unique_address]]
#endif
  union_type storage;

  index_type tag{npos};

  template <std::size_t Idx>
  constexpr static auto member = util::TypeList<Ptrs...>::template get<Idx>::value;

public:
  template <std::size_t Idx, typename... Args>
  constexpr explicit StorageProxy(std::in_place_index_t<Idx>, Args&&... args) {
    std::construct_at(std::addressof(storage.*member<Idx>), std::forward<Args>(args)...);
    tag = Idx;
  }

  constexpr StorageProxy(StorageProxy const& other)      = default;
  constexpr StorageProxy(StorageProxy&& other) noexcept  = default;
  StorageProxy& operator=(StorageProxy const& other)     = default;
  StorageProxy& operator=(StorageProxy&& other) noexcept = default;

  constexpr StorageProxy() {}
  constexpr ~StorageProxy()
    requires std::is_trivially_destructible_v<union_type>
  = default;
  constexpr ~StorageProxy() { reset(); }

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  constexpr void reset() {
    if (tag != npos) {
      slo::visit([](auto&& obj) { std::destroy_at(std::addressof(obj)); }, *this);
      tag = npos;
    }
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(std::addressof(storage.*member<Idx>), std::forward<Args>(args)...);
    tag = Idx;
  }

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args) {
    emplace<alternatives::template get_index<T>>(std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.*member<Idx>;
  }
  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return tag == npos; }
};
}  // namespace detail

template <auto... Ptrs>
using StorageProxy = detail::StorageProxy<detail::MemberPtr<Ptrs>...>;
}  // namespace slo::impl

#undef SLO_UNION_ATTR
