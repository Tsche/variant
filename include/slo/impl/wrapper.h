#pragma once
#include <concepts>
#include <cstddef>
#include <utility>

namespace slo::impl {
template <typename T>
concept has_index = requires(T const& obj) {
  { obj.index() } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept has_unwrap = requires(T obj) {
  { obj.unwrap() } -> std::same_as<typename T::type&>;
  { std::as_const(obj).unwrap() } -> std::same_as<typename T::type const&>;
  { std::move(obj).unwrap() } -> std::same_as<typename T::type&&>;
  { std::move(std::as_const(obj)).unwrap() } -> std::same_as<typename T::type const&&>;
};

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

static_assert(has_unwrap<Wrapper<0, int>>);

}  // namespace slo::impl