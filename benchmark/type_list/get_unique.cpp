#include <concepts>
#include <utility>

template <typename... Ts>
struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);
};

namespace fold {

template <typename T, typename List,
          typename = std::make_index_sequence<List::size>>
struct type_index;

template <typename T, typename... Ts, std::size_t... Idx>
struct type_index<T, TypeList<Ts...>, std::index_sequence<Idx...>> {
    constexpr static std::size_t occurrences = (std::same_as<T, Ts> + ... + 0);
    constexpr static std::size_t value =
        (occurrences != 1)
            ? sizeof...(Ts)
            : ((Idx * static_cast<std::size_t>(std::same_as<T, Ts>))+ ... + 0);
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
struct UniqueHelper<TypeList<Ts...>, std::index_sequence<Idx...>>
    : Indexed<Idx, Ts>... {};

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
    std::size_t result = sizeof...(Ts);

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

template <auto>
struct Dummy {};

template <std::size_t... Idx>
void run(std::index_sequence<Idx...>) {
#if defined(INHERITANCE)
  using inheritance::index;
#elif defined(FOLD)
  using fold::index;
#elif defined(ARRAY)
  using array::index;
#else
#error "Define either INHERITANCE, FOLD or ARRAY."
#endif
  static_assert(index<int, Dummy<Idx>...> == sizeof...(Idx));
  static_assert(((index<Dummy<Idx>, Dummy<Idx>...> == Idx) && ...));
  static_assert(index<float, float, Dummy<Idx>..., float> == sizeof...(Idx) + 2);
}

#ifndef COUNT
#define COUNT 10
#endif

int main() {
  run(std::make_index_sequence<COUNT>{});
}