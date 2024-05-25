#include <concepts>
#include <utility>

template <typename... Ts>
struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);
};

namespace fold {

template <typename T, typename List, typename = std::make_index_sequence<List::size>>
struct type_index;

template <typename T, typename... Ts, std::size_t... Idx>
struct type_index<T, TypeList<Ts...>, std::index_sequence<Idx...>> {
  constexpr static std::size_t occurrences = (std::same_as<T, Ts> + ... + 0);
  constexpr static std::size_t value =
      (occurrences != 1) ? sizeof...(Ts) : ((Idx * static_cast<std::size_t>(std::same_as<T, Ts>)) + ... + 0);
};

template <typename T, typename... Ts>
constexpr std::size_t index = type_index<T, TypeList<Ts...>>::value;
}  // namespace fold

namespace inheritance {
template <typename T>
struct Wrap {};

template <std::size_t I, typename T>
struct Indexed : Wrap<T> {};

template <typename T, typename = std::make_index_sequence<T::size>>
struct UniqueHelper;

template <typename... Ts, std::size_t... Idx>
struct UniqueHelper<TypeList<Ts...>, std::index_sequence<Idx...>> : Indexed<Idx, Ts>... {};

template <typename T, typename... Ts>
static constexpr bool occurs_once = std::is_convertible_v<UniqueHelper<TypeList<Ts...>>, Wrap<T>>;

template <typename T, typename... Ts>
consteval std::size_t get_type_index() {
  std::size_t index = 0;
  (void)((!std::same_as<T, Ts> ? ++index, false : true) || ...);
  return index;
}

template <typename, typename... Ts>
inline constexpr std::size_t index = sizeof...(Ts);

template <typename T, typename... Ts>
  requires occurs_once<T, Ts...>
inline constexpr std::size_t index<T, Ts...> = get_type_index<T, Ts...>();
}  // namespace inheritance

namespace array {

template <typename T, typename... Ts>
constexpr std::size_t get_index() {
  constexpr bool matches[] = {std::same_as<T, Ts>...};
  std::size_t result       = sizeof...(Ts);

  for (std::size_t idx = 0; idx < sizeof...(Ts); ++idx) {
    if (matches[idx]) {
      if (result != sizeof...(Ts)) {
        return sizeof...(Ts);
      }
      result = idx;
    }
  }
  return result;
}

template <typename T, typename... Ts>
inline constexpr std::size_t index = get_index<T, Ts...>();
}  // namespace array

namespace hash {
template <typename T>
constexpr auto hash_type() {
#if defined(_MSC_VER)
  return __FUNCSIG__;
#else
  return __PRETTY_FUNCTION__;
#endif
}

template <std::size_t N, typename U>
constexpr std::size_t get_index(U needle, U const (&haystack)[N]) {
  std::size_t result = N;
  for (std::size_t idx = 0; idx < N; ++idx) {
    if (needle == haystack[idx]) {
      if (result != N) {
        // ambiguous
        return N;
      }
      result = idx;
    }
  }
  return result;
}

template <typename T, typename... Ts>
inline constexpr std::size_t index = get_index(hash_type<T>, {hash_type<Ts>...});
}  // namespace hash

namespace inheritance2 {
namespace detail {
template <std::size_t Idx, typename T>
struct Tagged {
  using type                         = T;
  constexpr static std::size_t value = Idx;
};

template <typename, typename...>
struct GetHelper;
template <typename... T>
constexpr inline bool dependent_false = false;
template <std::size_t... Idx, typename... Ts>
struct GetHelper<std::index_sequence<Idx...>, Ts...> : Tagged<Idx, Ts>... {};

template <typename T, std::size_t Idx>
Tagged<Idx, T> rget_impl(Tagged<Idx, T>){
    static_assert(false, "get_index_impl not allowed in an evaluated context");
}
}  // namespace detail

template <typename T, typename... Ts>
concept occurs_once =
    requires(detail::GetHelper<std::index_sequence_for<Ts...>, Ts...> obj) { detail::rget_impl<T>(obj); };

template <typename T, typename... Ts>
constexpr inline std::size_t index = sizeof...(Ts);

template <typename T, typename... Ts>
  requires occurs_once<T, Ts...>
constexpr inline std::size_t index<T, Ts...> =
    decltype(detail::rget_impl<T>(detail::GetHelper<std::index_sequence_for<Ts...>, Ts...>{}))::value;
}  // namespace inheritance2

namespace inheritance3 {
template <typename T>
struct Wrap {};

template <std::size_t I, typename T>
struct Indexed : Wrap<T> {
  static constexpr std::size_t value = I;
};

template <typename T, typename = std::make_index_sequence<T::size>>
struct UniqueHelper;

template <typename... Ts, std::size_t... Idx>
struct UniqueHelper<TypeList<Ts...>, std::index_sequence<Idx...>> : Indexed<Idx, Ts>... {};

template <typename T, typename... Ts>
concept occurs_once = requires(UniqueHelper<TypeList<Ts...>> obj) { static_cast<Wrap<T>>(obj); };

template <typename T, std::size_t Idx>
Indexed<Idx, T> get_index_impl(Indexed<Idx, T>);

template <typename, typename... Ts>
inline constexpr std::size_t index = sizeof...(Ts);

template <typename T, typename... Ts>
  requires occurs_once<T, Ts...>
inline constexpr std::size_t index<T, Ts...> = decltype(get_index_impl<T>(UniqueHelper<TypeList<Ts...>>{}))::value;

}  // namespace inheritance3

namespace nested {
  namespace detail {
template <std::size_t Idx, typename T>
struct Tagged {
  using type                         = T;
  constexpr static std::size_t value = Idx;
};

template <typename T, std::size_t Idx>
constexpr Tagged<Idx, T> get_index_impl(Tagged<Idx, T>){
    static_assert(false, "get_index_impl not allowed in an evaluated context");
}
}  // namespace detail

template <typename... Ts>
struct TypeList2 {
  template <typename = std::index_sequence_for<Ts...>>
  struct GetHelper;

  template <std::size_t... Idx>
  struct GetHelper<std::index_sequence<Idx...>> : detail::Tagged<Idx, Ts>... {};

  template <typename T>
  constexpr static std::size_t index_of = sizeof...(Ts);

  template <typename T>
    requires requires(GetHelper<> obj) { detail::get_index_impl<T>(obj); }
  constexpr static std::size_t index_of<T> = decltype(detail::get_index_impl<T>(GetHelper{}))::value;
};

template <typename T, typename... Ts>
constexpr inline std::size_t index = TypeList2<Ts...>::template index_of<T>;
}

template <auto>
struct Dummy {};

template <std::size_t... Idx>
void run(std::index_sequence<Idx...>) {
#if defined(INHERITANCE)
  using inheritance::index;
#elif defined(INHERITANCE2)
  using inheritance2::index;
#elif defined(INHERITANCE3)
  using inheritance3::index;
#elif defined(NESTED)
  using nested::index;
#elif defined(FOLD)
  using fold::index;
#elif defined(ARRAY)
  using array::index;
#elif defined(HASH)
  using hash::index;
#else
#  error "Define either INHERITANCE, INHERITANCE2, INHERITANCE3, NESTED, FOLD, ARRAY or HASH."
#endif
  static_assert(index<int, Dummy<Idx>...> == sizeof...(Idx));
  static_assert(((index<Dummy<Idx>, Dummy<Idx>...> == Idx) && ...));
  static_assert(index<float, float, Dummy<Idx>..., float> == sizeof...(Idx) + 2);
}

#ifndef COUNT
#  define COUNT 10
#endif

int main() {
  run(std::make_index_sequence<COUNT>{});
}