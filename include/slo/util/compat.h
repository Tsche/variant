#pragma once
#include <utility>
#include <type_traits>

namespace slo::compat {
// #if __cpp_lib_forward_like < 202207L
template <typename T, typename U>
[[nodiscard]] constexpr auto&& forward_like(U&& obj) noexcept {
  if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
    if constexpr (std::is_lvalue_reference_v<T&&>) {
      return static_cast<std::remove_reference_t<U> const&>(obj);
    } else {
      return static_cast<std::remove_reference_t<U> const&&>(obj);
    }
  } else {
    if constexpr (std::is_lvalue_reference_v<T&&>) {
      return static_cast<U&>(obj);
    } else {
      return static_cast<std::remove_reference_t<U>&&>(obj);
    }
  }
}
// #else
// using std::forward_like;
// #endif

// standard libraries don't expose this one but it is rather handy
template <typename T, typename U>
using like_t = decltype(forward_like<T>(std::declval<U>()));

#if __cpp_lib_within_lifetime < 202306L
template <class T>
constexpr bool is_within_lifetime(const T* ptr) noexcept {
  // this is only constexpr because clang does not like it being consteval
#  if __has_builtin(__builtin_constant_p)
#    define HAS_IS_WITHIN_LIFETIME true
  return __builtin_constant_p(*ptr);
#  else
#    define HAS_IS_WITHIN_LIFETIME false
  return 0;
#  endif
}
#else
#  define HAS_IS_WITHIN_LIFETIME true
using std::is_within_lifetime;
#endif
}  // namespace slo::compat