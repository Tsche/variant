#pragma once
#include <cstddef>
#include <algorithm>
#include <string_view>

template <std::size_t N>
struct [[nodiscard]] const_string {
  char value[N + 1]{};
  constexpr static auto size = N;

  constexpr const_string() = default;

  constexpr explicit(false) const_string(char const (&literal)[N + 1]) {  // NOLINT
    std::copy(literal, literal + N, std::begin(value));
  }

  constexpr explicit const_string(std::string_view data) { std::copy(begin(data), end(data), std::begin(value)); }
  [[nodiscard]] constexpr explicit operator std::string_view() const noexcept { return std::string_view{value}; }
  [[nodiscard]] constexpr explicit operator char const*() const noexcept { return value; }

  [[nodiscard]] constexpr auto to_sv() const noexcept { return std::string_view{value}; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size == 0; }

  [[nodiscard]] constexpr char const& operator[](std::size_t Idx) const { return value[Idx]; }
  friend constexpr auto operator==(const_string lhs, std::string_view rhs) noexcept { return rhs == lhs.value; }
  constexpr auto length() const { return std::char_traits<char>::length(value); }
};

template <std::size_t N>
const_string(char const (&)[N]) -> const_string<N - 1>;

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
