#pragma once
#include <concepts>
#include <utility>

namespace slo::impl {
template <typename T>
concept has_get = requires(T obj) { obj.template get<0>(); };

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

}