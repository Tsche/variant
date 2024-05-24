#include <cstddef>
#include <type_traits>
#include <utility>
#include <concepts>

namespace recursive {
template <std::size_t Idx, typename T, typename... Ts>
struct GetImpl {
  using type = typename GetImpl<Idx - 1, Ts...>::type;
};

template <typename T, typename... Ts>
struct GetImpl<0, T, Ts...> {
  using type = T;
};

template <std::size_t Idx, typename... Ts>
using get = GetImpl<Idx, Ts...>::type;
}  // namespace recursive

namespace inheritance1 {
template <std::size_t Idx, typename T>
struct Tagged {
  using type = T;
};

template <typename, typename...>
struct GetHelper;

template <std::size_t... Idx, typename... Ts>
struct GetHelper<std::index_sequence<Idx...>, Ts...> : Tagged<Idx, Ts>... {};

template <std::size_t Idx, typename T>
Tagged<Idx, T> get_impl(Tagged<Idx, T>);

template <std::size_t Idx, typename... Ts>
using get = typename decltype(get_impl<Idx>(GetHelper<std::index_sequence_for<Ts...>, Ts...>{}))::type;
}  // namespace inheritance1

namespace inheritance2 {
template <std::size_t Idx, typename T>
struct Tagged {
  using type = T;
  Tagged operator()(std::integral_constant<std::size_t, Idx>);
};

template <typename, typename...>
struct GetHelper;

template <std::size_t... Idx, typename... Ts>
struct GetHelper<std::index_sequence<Idx...>, Ts...> : Tagged<Idx, Ts>... {
  using Tagged<Idx, Ts>::operator()...;
};

template <std::size_t Idx, typename... Ts>
using get = decltype(GetHelper<std::index_sequence_for<Ts...>, Ts...>{}(std::integral_constant<std::size_t, Idx>{}))::type;
}  // namespace inheritance2

namespace voidptr {
template <std::size_t N, typename = std::make_index_sequence<N>>
struct GetImpl;

template <std::size_t N, std::size_t... Skip>
struct GetImpl<N, std::index_sequence<Skip...>> {
  template <typename T>
  auto operator()(decltype((void*)Skip)..., T*, ...) -> T;
};

template <std::size_t Idx, typename... Ts>
using get = decltype(GetImpl<Idx>{}(static_cast<Ts*>(0)...));
}  // namespace voidptr

namespace ignored {
struct Universal {
    constexpr Universal() = default;
    constexpr Universal(auto&&) {}
    constexpr Universal& operator=(auto&&) { return *this; }
};

template <std::size_t N, typename = std::make_index_sequence<N>>
struct GetImpl;

template <std::size_t N, std::size_t... Skip>
struct GetImpl<N, std::index_sequence<Skip...>> {
    template <typename T>
    auto operator()(decltype(Universal{Skip})..., T&&, auto&&...) -> T;
};

template <std::size_t Idx, typename... Ts>
using get = decltype(GetImpl<Idx>{}(std::declval<Ts>()...));
}  // namespace ignored


namespace paging {
template <std::size_t Idx, typename T>
struct GetImpl;

template <template <typename...> class List, typename T0, typename... Ts>
struct GetImpl<0, List<T0, Ts...>> {
  using type = T0;
  using tail = List<Ts...>;
};

template <template <typename...> class List, typename T0, typename T1, typename... Ts>
struct GetImpl<1, List<T0, T1, Ts...>> {
  using type = T1;
  using tail = List<Ts...>;
};

template <template <typename...> class List, typename T0, typename T1, typename T2, typename... Ts>
struct GetImpl<2, List<T0, T1, T2, Ts...>> {
  using type = T2;
  using tail = List<Ts...>;
};

template <template <typename...> class List, typename T0, typename T1, typename T2, typename T3, typename... Ts>
struct GetImpl<3, List<T0, T1, T2, T3, Ts...>> {
  using type = T3;
  using tail = List<Ts...>;
};


template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename... Ts>
struct GetImpl<4, List<T0, T1, T2, T3, T4, Ts...>> {
  using type = T4;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename... Ts>
struct GetImpl<5, List<T0, T1, T2, T3, T4, T5, Ts...>> {
  using type = T5;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename... Ts>
struct GetImpl<6, List<T0, T1, T2, T3, T4, T5, T6, Ts...>> {
  using type = T6;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename... Ts>
struct GetImpl<7, List<T0, T1, T2, T3, T4, T5, T6, T7, Ts...>> {
  using type = T7;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename... Ts>
struct GetImpl<8, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, Ts...>> {
  using type = T8;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename... Ts>
struct GetImpl<9, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, Ts...>> {
  using type = T9;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename T10,
          typename... Ts>
struct GetImpl<10, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, Ts...>> {
  using type = T10;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename T10,
          typename T11,
          typename... Ts>
struct GetImpl<11, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, Ts...>> {
  using type = T11;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename T10,
          typename T11,
          typename T12,
          typename... Ts>
struct GetImpl<12, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, Ts...>> {
  using type = T12;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename T10,
          typename T11,
          typename T12,
          typename T13,
          typename... Ts>
struct GetImpl<13, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, Ts...>> {
  using type = T13;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename T10,
          typename T11,
          typename T12,
          typename T13,
          typename T14,
          typename... Ts>
struct GetImpl<14, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, Ts...>> {
  using type = T14;
  using tail = List<Ts...>;
};

template <template <typename...> class List,
          typename T0,
          typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8,
          typename T9,
          typename T10,
          typename T11,
          typename T12,
          typename T13,
          typename T14,
          typename T15,
          typename... Ts>
struct GetImpl<15, List<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, Ts...>> {
  using type = T15;
  using tail = List<Ts...>;
};

constexpr inline auto PAGE_SIZE = 15U;

template <std::size_t Idx, template <typename...> class List, typename... Ts>
  requires(Idx >= PAGE_SIZE)
struct GetImpl<Idx, List<Ts...>> {
  using first_page = GetImpl<PAGE_SIZE - 1, List<Ts...>>;
  using recurse    = GetImpl<Idx - PAGE_SIZE, typename first_page::tail>;
  using type       = typename recurse::type;
  using tail       = typename recurse::tail;
};

template <typename...>
struct List {};
template <std::size_t Idx, typename... Ts>
using get = GetImpl<Idx, List<Ts...>>::type;
}  // namespace paging

#if __has_builtin(__type_pack_element)
namespace builtin {
template <std::size_t Idx, typename... Ts>
using get = __type_pack_element<Idx, Ts...>;
}  // namespace builtin
#endif

namespace cpp26 {
template <std::size_t Idx, typename... Ts>
using get = Ts...[Idx];
}  // namespace cpp26

template <auto>
struct Dummy {};

template <std::size_t... Idx>
void run(std::index_sequence<Idx...>) {
#if defined(RECURSIVE)
  using recursive::get;
#elif defined(INHERITANCE1)
  using inheritance1::get;
#elif defined(INHERITANCE2)
  using inheritance2::get;
#elif defined(VOIDPTR)
  using voidptr::get;
#elif defined(IGNORED)
  using ignored::get;
#elif defined(PAGING)
  using paging::get;
#elif defined(BUILTIN)
#  if !__has_builtin(__type_pack_element)
#    error "Your compiler does not have a pack indexing builtin"
#  endif
  using builtin::get;
#elif defined(CPP26)
// #  if __cpp_pack_indexing < 202311L
// #    error "Your compiler does not implement C++26 pack indexing"
// #  endif
  using cpp26::get;
#else
#  error "Define one of RECURSIVE, INHERITANCE1, INHERITANCE2, VOIDPTR, PAGING, BUILTIN, CPP26."
#endif
  static_assert((std::same_as<get<Idx, Dummy<Idx>...>, Dummy<Idx>> && ...));
}

#ifndef COUNT
#  define COUNT 10
#endif

int main() {
  run(std::make_index_sequence<COUNT>{});
}