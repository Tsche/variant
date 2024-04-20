#pragma once
#include <cstddef>
#include <concepts>
#include <utility>
#include <type_traits>

namespace slo {
namespace impl {
template <typename T, typename... Ts>
consteval std::size_t get_type_index() {
  std::size_t index = 0;
  (void)((!std::same_as<T, Ts> ? ++index, false : true) || ...);
  return index;
}

template <typename T, typename... Ts>
constexpr inline std::size_t type_index = get_type_index<std::remove_cvref_t<T>, Ts...>();

template <typename T>
concept has_get = requires(T obj) { obj.template get<0>(); };
}  // namespace impl

template <std::size_t, typename>
struct variant_alternative;


template <typename... Ts>
class Variant;

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, Variant<Ts...>> {
  using type = __type_pack_element<Idx, Ts...>;
};

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, Variant<Ts...> const> {
  using type = __type_pack_element<Idx, Ts...> const;
};


template <typename... Ts>
union Union;

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, Union<Ts...>> {
  using type = __type_pack_element<Idx, Ts...>;
};

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, Union<Ts...> const> {
  using type = std::add_const_t<__type_pack_element<Idx, Ts...>>;
};

template <std::size_t Idx, typename V>
using variant_alternative_t = typename variant_alternative<Idx, V>::type;

template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<Idx>();
}

template <typename T, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<T>();
}

struct nullopt {};

}  // namespace slo