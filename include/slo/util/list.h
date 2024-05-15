#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "slo/util/pack.h"

namespace slo::util {

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