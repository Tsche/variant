#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace slo::util {

namespace detail {
template <std::size_t Idx, typename T>
struct Tagged {
  using type                         = T;
  constexpr static std::size_t value = Idx;
};

template <std::size_t Idx, typename T>
constexpr T get_type_impl(Tagged<Idx, T>) {
  static_assert(false, "get_type_impl not allowed in an evaluated context");
}

template <typename T, std::size_t Idx>
constexpr Tagged<Idx, T> get_index_impl(Tagged<Idx, T>) {
  static_assert(false, "get_index_impl not allowed in an evaluated context");
}

}  // namespace detail

template <typename... Ts>
struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);

  template <typename = std::index_sequence_for<Ts...>>
  struct GetHelper;

  template <std::size_t... Idx>
  struct GetHelper<std::index_sequence<Idx...>> : detail::Tagged<Idx, Ts>... {};

  /**
   * @brief Aliases the type at the specified index
   *
   * @tparam Idx
   */
  template <std::size_t Idx>
  using type_at =
#if __has_builtin(__type_pack_element)
      __type_pack_element<Idx, Ts...>;
#else
      decltype(get_type_impl<Idx>(GetHelper{}));
#endif

  /**
   * @brief Reverse mapping type -> index. Returns sizeof...(Ts) if the type could not be found
   *        or it appeared more than once in the list.
   *
   * @tparam T
   */
  template <typename T>
  constexpr static std::size_t index_of = sizeof...(Ts);

  template <typename T>
  // constraint is not fulfilled if T is contained more than once (=> get_index_impl would be ambiguous)
    requires requires(GetHelper<> obj) { detail::get_index_impl<T>(obj); }
  constexpr static std::size_t index_of<T> = decltype(detail::get_index_impl<T>(GetHelper<>{}))::value;

  using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;

  template <template <typename> class Trait>
  constexpr static bool all = (Trait<Ts>::value && ...);

  template <template <typename> class Trait>
  constexpr static bool any = (Trait<Ts>::value || ...);
};

template <std::size_t Idx, typename List>
using type_at = typename List::template type_at<Idx>;

template <typename T, typename List>
constexpr inline std::size_t index_of = List::template index_of<T>;
}  // namespace slo::util