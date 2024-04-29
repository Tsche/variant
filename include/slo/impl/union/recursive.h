#pragma once
#include <utility>
#include <slo/impl/wrapper.h>
#include <slo/util/compat.h>

namespace slo::impl {
template <typename... Ts>
union RecursiveUnion;

template <typename T>
union RecursiveUnion<T> {
  T value;
  constexpr ~RecursiveUnion() {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args) : value{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const
    requires(has_index<T>)
  {
    return value.tag;
  }

  template <std::size_t N, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    static_assert(N == 0);
    if constexpr (has_unwrap<T>) {
      return std::forward<Self>(self).value.unwrap();
    } else {
      return compat::forward_like<Self>(std::forward<Self>(self).value);
    }
  }
};

template <typename T0, typename... Ts>
  requires(sizeof...(Ts) != 0)
union RecursiveUnion<T0, Ts...> {
  T0 alternative_0;
  RecursiveUnion<Ts...> tail;

  constexpr ~RecursiveUnion() {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args)
      : alternative_0{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 1>{}, std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const
    requires(has_index<T0>)
  {
    if (compat::is_within_lifetime(&alternative_0.tag)) {
      return alternative_0.tag;
    } else {
      return tail.index();
    }
  }

  template <std::size_t N, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (N == 0) {
      if constexpr (has_unwrap<T0>) {
        return std::forward<Self>(self).alternative_0.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_0);
      }
    } else {
      return std::forward<Self>(self).tail.template get<N - 1>();
    }
  }
};

}  // namespace slo::impl