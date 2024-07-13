#include <type_traits>
#include <utility>
#include <slo/util/list.h>
#include <slo/impl/feature.h>
#define SLO_USE_PACK_INDEXING ON



#if USING(SLO_USE_PACK_INDEXING) || __cpp_pack_indexing >= 202311L
#  define SLO_TYPE_AT(Idx, Pack) Pack[Idx]
#define SLO_HAS_TYPE_PACK_INDEXING 1
#elif __has_builtin(__type_pack_element)
#  define SLO_TYPE_AT(Idx, Pack) __type_pack_element<Idx, Pack>
#define SLO_HAS_TYPE_PACK_INDEXING 0
#else
#define SLO_HAS_TYPE_PACK_INDEXING 0
# define SLO_TYPE_AT(Idx, Pack) slo::util::type_at<Idx, slo::util::TypeList<Pack>>
#endif

template <typename...>
union UnionType {};

namespace indexing {
namespace detail {

template <template <typename...> class, typename, typename...>
struct Tree;

template <template <typename...> class Node, std::size_t... I, typename T>
struct Tree<Node, std::index_sequence<I...>, T> {
  using type = Node<T>;
};

template <template <typename...> class Node, typename... T>
struct Tree<Node, std::index_sequence<>, Node<T...>> {
  using type = Node<T...>;
};

template <template <typename...> class Node, std::size_t... I, typename... T>
  requires(sizeof...(T) > 1 && sizeof...(T) % 2 == 0)
struct Tree<Node, std::index_sequence<I...>, T...> {
  using type = Tree<Node,
                    std::make_index_sequence<sizeof...(T) / 4>,
                    Node<SLO_TYPE_AT(2 * I, T...), SLO_TYPE_AT(2 * I + 1, T...)>...>::type;
};

template <template <typename...> class Node, std::size_t... I, typename... T>
  requires(sizeof...(T) > 1 && sizeof...(T) % 2 != 0)
struct Tree<Node, std::index_sequence<I...>, T...> {
  using type = Tree<Node,
                    std::make_index_sequence<(sizeof...(T) + 2) / 4>,
                    Node<SLO_TYPE_AT(2 * I, T...), SLO_TYPE_AT(2 * I + 1, T...)>...,
                    SLO_TYPE_AT(sizeof...(T) - 1, T...)>::type;
};

}  // namespace detail

template <template <typename...> class Node, typename... Ts>
using to_cbt = detail::Tree<Node, std::make_index_sequence<sizeof...(Ts) / 2>, Ts...>::type;
}  // namespace indexing

namespace recursive {
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

template <template <typename...> class Node, typename T1, typename T2, typename... In, typename... Out>
struct ToCBTImpl<Node, TypeList<T1, T2, In...>, TypeList<Out...>> {
  using type = ToCBTImpl<Node, TypeList<In...>, TypeList<Out..., Node<T1, T2>>>::type;
};
}  // namespace detail

template <template <typename...> class Node, typename... Ts>
using to_cbt = typename detail::ToCBTImpl<Node, TypeList<Ts...>, TypeList<>>::type;
}  // namespace recursive

template <auto>
struct Constant {};

template <typename...>
union Node {};

template <std::size_t... Idx>
auto generate_tree(std::index_sequence<Idx...>) {
#if defined(INDEXING)
  using underlying = indexing::to_cbt<UnionType, Constant<Idx>...>;
#elif defined(RECURSIVE)
  using underlying = recursive::to_cbt<UnionType, Constant<Idx>...>;
#else
#  error
#endif
  return std::type_identity<underlying>{};
}

int main() {
  using TestType = typename decltype(generate_tree(std::make_index_sequence<COUNT>()))::type;
}
