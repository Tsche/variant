#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "slo/util/pack.h"

namespace slo::util {

template <typename... Ts>
struct TypeList;

namespace detail {
template <template <typename...> class, typename, typename>
struct ToCBTImpl;

template <template <typename...> class Node, typename T>
struct ToCBTImpl<Node, TypeList<T>, TypeList<>> {
    using type = Node<T>;
};

template <template <typename...> class Node, typename T, typename U>
struct ToCBTImpl<Node, TypeList<>, TypeList<Node<T, U>>> {
    using type = Node<T, U>;
};

template <template <typename...> class Node, typename... Out>
struct ToCBTImpl<Node, TypeList<>, TypeList<Out...>> {
    using type = ToCBTImpl<Node, TypeList<Out...>, TypeList<>>::type;
};

template <template <typename...> class Node, typename T, typename... Out>
struct ToCBTImpl<Node, TypeList<T>, TypeList<Out...>> {
    using type = ToCBTImpl<Node, TypeList<Out..., T>, TypeList<>>::type;
};

template <template <typename...> class Node, typename T1, typename T2,
          typename... In, typename... Out>
struct ToCBTImpl<Node, TypeList<T1, T2, In...>, TypeList<Out...>> {
    using type =
        ToCBTImpl<Node, TypeList<In...>, TypeList<Out..., Node<T1, T2>>>::type;
};
}  // namespace detail

template <template <typename...> class Node, typename... Ts>
using to_cbt = typename detail::ToCBTImpl<Node, TypeList<Ts...>, TypeList<>>::type;

template <typename T, typename... Ts>
consteval std::size_t get_type_index() {
  std::size_t index = 0;
  (void)((!std::same_as<T, Ts> ? ++index, false : true) || ...);
  return index;
}

template <typename T, typename... Ts>
constexpr inline std::size_t type_index = get_type_index<std::remove_cvref_t<T>, Ts...>();

template <typename... Ts>
struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);

  template <std::size_t Idx>
  using get = pack::get<Idx, TypeList>;

  template <typename T>
  constexpr static std::size_t get_index = type_index<T, Ts...>;

  using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;
};

}  // namespace slo::util