#pragma once
#include <cstddef>
#include <string_view>
#include "const_string.h"

namespace detail {
template <typename T>
constexpr auto get_ctti() {
#if defined(_MSC_VER)
  constexpr auto prefix = std::string_view{"get_ctti<"};
  constexpr auto suffix = std::string_view{">(void)"};
  constexpr auto signature    = std::string_view{__FUNCSIG__};
#else
  constexpr auto prefix = std::string_view{"T = "};
  constexpr auto suffix = std::string_view{"]"};
  constexpr auto signature    = std::string_view{__PRETTY_FUNCTION__};
#endif

  constexpr std::size_t start = signature.find(prefix) + prefix.size();
  constexpr std::size_t end   = signature.find(suffix, start);

  constexpr auto value = signature.substr(start, end - start);
  return const_string<value.size()>(value);
}

template <typename T>
constexpr inline auto raw_type_name = get_ctti<T>();

}  // namespace detail

template <typename T>
constexpr inline auto nameof = std::string_view{detail::raw_type_name<T>};
