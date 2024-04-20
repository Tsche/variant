#pragma once
#include <concepts>
#include <type_traits>

namespace slo::util {
template <typename T, typename... Ts>
concept all_same = (std::same_as<T, Ts> && ...);

template <typename T>
concept is_enum = std::is_enum_v<T>;

template <typename T>
concept is_class = std::is_class_v<T>;

template <typename T>
concept is_union = std::is_union_v<T>;
}