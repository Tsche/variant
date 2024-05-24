#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "slo/util/pack.h"

namespace slo::util {
template <typename... Ts>
struct TypeList {
private:
  template <typename T>
  static constexpr std::size_t get_index_impl() {
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

public:
  static constexpr std::size_t size = sizeof...(Ts);

  template <std::size_t Idx>
  using get = pack::get<Idx, TypeList>;

  /**
   * @brief Reverse mapping type -> index. Returns sizeof...(Ts) if the type could not be found
   *        or it appeared more than once in the list.
   *
   * @tparam T
   */
  template <typename T>
  constexpr static std::size_t get_index = get_index_impl<T>();

  using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;

  template <template <typename> class Trait>
  constexpr static bool all = (Trait<Ts>::value && ...);

  template <template <typename> class Trait>
  constexpr static bool any = (Trait<Ts>::value || ...);
};

}  // namespace slo::util