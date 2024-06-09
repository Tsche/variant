#pragma once
#include <cstddef>
#include <utility>
#include <memory>

#include <slo/impl/feature.h>
#include <slo/impl/union/recursive.h>
#include <slo/impl/union/tree.h>
#include <slo/util/list.h>
#include <slo/util/utility.h>

namespace slo {
template <typename F, typename... Vs>
constexpr decltype(auto) visit(F&& visitor, Vs&&... variant);
}

namespace slo::impl {
/**
 * @brief Essentially a plain discriminated union.
 *
 * @tparam Ts
 */
template <template <typename...> class Generator, typename... Ts>
class Storage {
  // discriminated union
public:
  using alternatives               = util::TypeList<Ts...>;
  using index_type                 = alternatives::index_type;
  constexpr static index_type npos = static_cast<index_type>(~0U);

private:
  using union_type = Generator<Ts...>;
  union {
    util::error_type dummy = {};
    union_type value;
  };

  index_type tag{npos};

public:
  template <std::size_t Idx, typename... Args>
  constexpr explicit Storage(std::in_place_index_t<Idx> idx, Args&&... args)
      : value(idx, std::forward<Args>(args)...)
      , tag(Idx) {}

  constexpr Storage() {}
  constexpr Storage(Storage const& other)                = default;
  constexpr Storage(Storage&& other) noexcept            = default;
  constexpr Storage& operator=(Storage const& other)     = default;
  constexpr Storage& operator=(Storage&& other) noexcept = default;

  constexpr ~Storage()
    requires std::is_trivially_destructible_v<union_type>
  = default;
  constexpr ~Storage() { reset(); }

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  constexpr void reset() {
    if (tag != npos) {
      slo::visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
      tag = npos;
    }
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&value, std::in_place_index<Idx>, std::forward<Args>(args)...);
    tag = Idx;
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).value.template get<Idx>();
  }
  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return tag == npos; }
};
}  // namespace slo::impl