#pragma once
#include <utility>
#include <slo/util/compat.h>
#include <slo/impl/concepts.h>

namespace slo::impl {
template <bool Trivial, typename... Ts>
union RecursiveUnion;

template <bool Trivial>
union RecursiveUnion<Trivial>{};

template <bool Trivial, typename T>
union RecursiveUnion<Trivial, T> {
  T value;

  constexpr RecursiveUnion(RecursiveUnion const&)            = default;
  constexpr RecursiveUnion(RecursiveUnion&&)                 = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion const&) = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion&&)      = default;
  constexpr ~RecursiveUnion()                                = default;
  constexpr ~RecursiveUnion() requires(!Trivial) {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args) : value{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const requires(has_index<T>) { return value.tag; }

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

template <bool Trivial, typename T0, typename... Ts>
requires(sizeof...(Ts) != 0) union RecursiveUnion<Trivial, T0, Ts...> {
  T0 alternative_0;
  RecursiveUnion<Trivial, Ts...> tail;

  constexpr RecursiveUnion(RecursiveUnion const&)            = default;
  constexpr RecursiveUnion(RecursiveUnion&&)                 = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion const&) = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion&&)      = default;
  constexpr ~RecursiveUnion()                                = default;
  constexpr ~RecursiveUnion() requires(!Trivial) {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args)
      : alternative_0{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 1>{}, std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const requires(has_index<T0>) {
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

template <bool Trivial, typename T0, typename T1, typename... Ts>
requires(sizeof...(Ts) != 0) union RecursiveUnion<Trivial, T0, T1, Ts...> {
  T0 alternative_0;
  T1 alternative_1;
  RecursiveUnion<Trivial, Ts...> tail;

  constexpr RecursiveUnion(RecursiveUnion const&)            = default;
  constexpr RecursiveUnion(RecursiveUnion&&)                 = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion const&) = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion&&)      = default;
  constexpr ~RecursiveUnion()                                = default;
  constexpr ~RecursiveUnion() requires(!Trivial) {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args)
      : alternative_0{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<1>, Args&&... args)
      : alternative_1{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 2>{}, std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const requires(has_index<T0>&& has_index<T1>) {
    if (compat::is_within_lifetime(&alternative_0.tag)) {
      return alternative_0.tag;
    } else if (compat::is_within_lifetime(&alternative_1.tag)) {
      return alternative_1.tag;
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
    } else if constexpr (N == 1) {
      if constexpr (has_unwrap<T1>) {
        return std::forward<Self>(self).alternative_1.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_1);
      }
    } else {
      return std::forward<Self>(self).tail.template get<N - 2>();
    }
  }
};

template <bool Trivial, typename T0, typename T1, typename T2, typename T3, typename... Ts>
requires(sizeof...(Ts) != 0) union RecursiveUnion<Trivial, T0, T1, T2, T3, Ts...> {
  T0 alternative_0;
  T1 alternative_1;
  T2 alternative_2;
  T3 alternative_3;
  RecursiveUnion<Trivial, Ts...> tail;

  constexpr RecursiveUnion(RecursiveUnion const&)            = default;
  constexpr RecursiveUnion(RecursiveUnion&&)                 = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion const&) = default;
  constexpr RecursiveUnion& operator=(RecursiveUnion&&)      = default;
  constexpr ~RecursiveUnion()                                = default;
  constexpr ~RecursiveUnion() requires(!Trivial) {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<0>, Args&&... args)
      : alternative_0{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<1>, Args&&... args)
      : alternative_1{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<2>, Args&&... args)
      : alternative_2{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<3>, Args&&... args)
      : alternative_3{std::forward<Args>(args)...} {}

  template <std::size_t N, typename... Args>
  constexpr explicit RecursiveUnion(std::in_place_index_t<N>, Args&&... args)
      : tail{std::in_place_index_t<N - 4>{}, std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const
      requires(has_index<T0>&& has_index<T1>&& has_index<T2>&& has_index<T3>) {
    if (compat::is_within_lifetime(&alternative_0.tag)) {
      return alternative_0.tag;
    } else if (compat::is_within_lifetime(&alternative_1.tag)) {
      return alternative_1.tag;
    } else if (compat::is_within_lifetime(&alternative_2.tag)) {
      return alternative_2.tag;
    } else if (compat::is_within_lifetime(&alternative_3.tag)) {
      return alternative_3.tag;
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
    } else if constexpr (N == 1) {
      if constexpr (has_unwrap<T1>) {
        return std::forward<Self>(self).alternative_1.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_1);
      }
    } else if constexpr (N == 2) {
      if constexpr (has_unwrap<T2>) {
        return std::forward<Self>(self).alternative_2.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_2);
      }
    } else if constexpr (N == 3) {
      if constexpr (has_unwrap<T3>) {
        return std::forward<Self>(self).alternative_3.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).alternative_3);
      }
    } else {
      return std::forward<Self>(self).tail.template get<N - 4>();
    }
  }
};

}  // namespace slo::impl