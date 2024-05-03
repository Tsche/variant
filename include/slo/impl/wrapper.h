#pragma once
#include <cstddef>
#include <utility>
// #include "concepts.h"

namespace slo::impl {
template <auto Idx, typename ValueType>
struct Wrapper {
  using type = ValueType;

  template <typename... Args>
  constexpr explicit Wrapper(Args&&... args) : tag{Idx}
                                             , storage{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  [[nodiscard]] constexpr ValueType& unwrap() & { return storage; }
  [[nodiscard]] constexpr ValueType const& unwrap() const& { return storage; }
  [[nodiscard]] constexpr ValueType&& unwrap() && { return std::move(storage); }
  [[nodiscard]] constexpr ValueType const&& unwrap() const&& { return std::move(storage); }

  decltype(Idx) tag;
  ValueType storage;
};

// static_assert(has_unwrap<Wrapper<0, int>>);

}  // namespace slo::impl