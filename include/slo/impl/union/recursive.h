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

  constexpr std::size_t index() const
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

template <typename T, typename... Ts>
union RecursiveUnion<T, Ts...> {
  T value;
  RecursiveUnion<Ts...> tail;

  constexpr ~RecursiveUnion() {}
  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args) : value{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 1>{}, std::forward<Args>(args)...} {}

  constexpr std::size_t index() const
    requires(has_index<T>)
  {
    if (compat::is_within_lifetime(&value.tag)) {
      return value.tag;
    }
    return tail.index();
  }

  template <std::size_t N, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (N == 0) {
      if constexpr (has_unwrap<T>) {
        return std::forward<Self>(self).value.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).value);
      }
    } else {
      return std::forward<Self>(self).tail.template get<N - 1>();
    }
  }
};

template <typename T1, typename T2, typename... Ts>
requires (sizeof...(Ts) != 0)
union RecursiveUnion<T1, T2, Ts...> {
  T1 alternative_1;
  T2 alternative_2;
  RecursiveUnion<Ts...> tail;

  constexpr ~RecursiveUnion() {}
  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args) : alternative_1{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<1>, Args&&... args) : alternative_2{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 2>{}, std::forward<Args>(args)...} {}

  constexpr std::size_t index() const
    requires (has_index<T1> && has_index<T2>)
  {
    if (compat::is_within_lifetime(&alternative_1.tag)) {
      return alternative_1.tag;
    } else if (compat::is_within_lifetime(&alternative_2.tag)) {
      return alternative_2.tag;
    }
    return tail.index();
  }

  template <std::size_t N, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (N == 0) {
      if constexpr (has_unwrap<T1>) {
        return std::forward<Self>(self).alternative_1.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_1);
      }
    } else if constexpr (N == 1) {
      if constexpr (has_unwrap<T2>) {
        return std::forward<Self>(self).alternative_2.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_2);
      }
    } else {
      return std::forward<Self>(self).tail.template get<N - 2>();
    }
  }
};

template <typename T1, typename T2, typename T3, typename T4, typename... Ts>
requires (sizeof...(Ts) != 0)
union RecursiveUnion<T1, T2, T3, T4, Ts...> {
  T1 alternative_1;
  T2 alternative_2;
  T3 alternative_3;
  T4 alternative_4;
  RecursiveUnion<Ts...> tail;

  constexpr ~RecursiveUnion() {}
  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args) : alternative_1{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<1>, Args&&... args) : alternative_2{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<2>, Args&&... args) : alternative_3{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<3>, Args&&... args) : alternative_4{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 4>{}, std::forward<Args>(args)...} {}

  constexpr std::size_t index() const
    requires (has_index<T1> && has_index<T2> && has_index<T3> && has_index<T4>)
  {
    if (compat::is_within_lifetime(&alternative_1.tag)) {
      return alternative_1.tag;
    } else if (compat::is_within_lifetime(&alternative_2.tag)) {
      return alternative_2.tag;
    } else if (compat::is_within_lifetime(&alternative_3.tag)) {
      return alternative_3.tag;
    } else if (compat::is_within_lifetime(&alternative_4.tag)) {
      return alternative_4.tag;
    }
    return tail.index();
  }

  template <std::size_t N, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (N == 0) {
      if constexpr (has_unwrap<T1>) {
        return std::forward<Self>(self).alternative_1.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_1);
      }
    } else if constexpr (N == 1) {
      if constexpr (has_unwrap<T2>) {
        return std::forward<Self>(self).alternative_2.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_2);
      }
    } else if constexpr (N == 2) {
      if constexpr (has_unwrap<T3>) {
        return std::forward<Self>(self).alternative_3.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_3);
      }
    } else if constexpr (N == 3) {
      if constexpr (has_unwrap<T4>) {
        return std::forward<Self>(self).alternative_4.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_4);
      }
    } else {
      return std::forward<Self>(self).tail.template get<N - 4>();
    }
  }
};
}  // namespace slo::impl