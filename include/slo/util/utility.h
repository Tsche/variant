#pragma once
#include <utility>

namespace slo::util {
struct error_type{};

template <typename>
struct in_place_tag : std::false_type {};

template <typename T>
struct in_place_tag<std::in_place_type_t<T>> : std::true_type {};
template <std::size_t V>
struct in_place_tag<std::in_place_index_t<V>> : std::true_type {};

template <typename T>
concept is_in_place = in_place_tag<T>::value;


template <bool, template <typename...> class T, template <typename...> class F>
struct ConditionalTemplate;

template <template <typename...> class T, template <typename...> class F>
struct ConditionalTemplate<true, T, F> {
  template <typename... Ts>
  using type = T<Ts...>;
};

template <template <typename...> class T, template <typename...> class F>
struct ConditionalTemplate<false, T, F> {
  template <typename... Ts>
  using type = F<Ts...>;
};
}  // namespace slo::util