#pragma once
#include <type_traits>

namespace slo::util {
template<typename...>
struct all_same : std::true_type{};

template<typename T, typename... Us>
struct all_same<T, Us...> : std::conjunction<std::is_same<T, Us>...>{};

template <typename... Ts>
constexpr inline bool all_same_v = all_same<Ts...>::value;

template <typename... Ts>
concept is_homogeneous = all_same<Ts...>::value;
}