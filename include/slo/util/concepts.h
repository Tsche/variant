#pragma once
#include <type_traits>
#include <functional>


namespace slo::util {
template<typename...>
struct all_same : std::true_type{};

template<typename T, typename... Us>
struct all_same<T, Us...> : std::conjunction<std::is_same<T, Us>...>{};

template <typename... Ts>
constexpr inline bool all_same_v = all_same<Ts...>::value;

template <typename... Ts>
concept is_homogeneous = all_same<Ts...>::value;

template <typename T>
concept is_hashable = 
    std::is_default_constructible_v<std::hash<T>>
    && std::is_copy_constructible_v<std::hash<T>>
    && std::is_move_constructible_v<std::hash<T>>
    && std::is_invocable_r_v<std::size_t, std::hash<T>, T const&>;
}