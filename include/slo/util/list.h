#pragma once
#include <utility>

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


template <typename... Ts>
struct TypeList {
  template <template <typename...> class TreeType>
  using as_cbt = to_cbt<TreeType, Ts...>;

  template <template <typename...> class T>
  using as = T<Ts...>;
};

namespace detail {
template <typename>
struct ToIndexSequence;

template <template<typename...> class T, typename... Ts>
struct ToIndexSequence<T<Ts...>>{ 
  using type = std::index_sequence_for<Ts...>;
};

template <template<bool, typename...> class T, typename... Ts>
struct ToIndexSequence<T<false, Ts...>>{ 
  // TODO
  using type = std::index_sequence_for<Ts...>;
};

}
template <typename T>
using to_index_sequence = typename detail::ToIndexSequence<std::remove_cvref_t<T>>::type;

}  // namespace slo::util