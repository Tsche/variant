#pragma once
#include <cstddef>
#include <utility>
#include <slo/impl/wrapper.h>
#include <slo/util/compat.h>

namespace slo::impl {
template <typename... Ts>
union TreeUnion;

template <typename... Ts, typename... Us>
union TreeUnion<TreeUnion<Ts...>, TreeUnion<Us...>> {
  constexpr static std::size_t size = TreeUnion<Ts...>::size + TreeUnion<Us...>::size;
  TreeUnion<Ts...> left;
  TreeUnion<Us...> right;

  constexpr ~TreeUnion() {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx> idx, Args&&... args)
    requires(Idx < TreeUnion<Ts...>::size)
      : left{idx, std::forward<Args>(args)...} {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx>, Args&&... args)
    requires(Idx >= TreeUnion<Ts...>::size)
      : right{std::in_place_index<Idx - TreeUnion<Ts...>::size>, std::forward<Args>(args)...} {}

  constexpr std::size_t index() const {
    if (compat::is_within_lifetime(&left)) {
      return left.index();
    }
    return right.index();
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (Idx < TreeUnion<Ts...>::size) {
      return std::forward<Self>(self).left.template get<Idx>();
    } else {
      return std::forward<Self>(self).right.template get<Idx - TreeUnion<Ts...>::size>();
    }
  }
};

template <typename... Ts, typename U>
union TreeUnion<TreeUnion<Ts...>, U> {
  constexpr static std::size_t size = 1 + TreeUnion<Ts...>::size;
  TreeUnion<Ts...> left;
  U right;

  constexpr ~TreeUnion() {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx> idx, Args&&... args)
    requires(Idx < TreeUnion<Ts...>::size)
      : left{idx, std::forward<Args>(args)...} {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx>, Args&&... args)
    requires(Idx == TreeUnion<Ts...>::size)
      : right{std::forward<Args>(args)...} {}

  constexpr std::size_t index() const {
    if (compat::is_within_lifetime(&right.tag)) {
      return right.tag;
    }
    return left.index();
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (Idx < TreeUnion<Ts...>::size) {
      return std::forward<Self>(self).left.template get<Idx>();
    } else {
      static_assert(Idx == TreeUnion<Ts...>::size, "Index out of bounds");
      if constexpr (has_unwrap<U>) {
        return std::forward<Self>(self).right.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).right);
      }
    }
  }
};

template <typename T, typename U>
union TreeUnion<T, U> {
  T left;
  U right;

  constexpr static std::size_t size = 2;

  constexpr ~TreeUnion() {}

  template <typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<0>, Args&&... args) : left{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<1>, Args&&... args) : right{std::forward<Args>(args)...} {}

  constexpr std::size_t index() const {
    if (compat::is_within_lifetime(&left.tag)) {
      return left.tag;
    } else if (compat::is_within_lifetime(&right.tag)) {
      return right.tag;
    }
    std::unreachable();
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (Idx == 0) {
      if constexpr (has_unwrap<T>) {
        return std::forward<Self>(self).left.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).left);
      }
    } else {
      static_assert(Idx == 1, "Index out of bounds");
      if constexpr (has_unwrap<U>) {
        return std::forward<Self>(self).right.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).right);
      }
    }
  }
};

template <typename T>
union TreeUnion<T> {
  T left;
  constexpr static std::size_t size = 1;

  constexpr ~TreeUnion() {}

  template <typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<0>, Args&&... args) : left{std::forward<Args>(args)...} {}

  constexpr std::size_t index() const {
    if (compat::is_within_lifetime(&left.tag)) {
      return left.tag;
    }
    std::unreachable();
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    static_assert(Idx == 0);
    if constexpr (has_unwrap<T>) {
      return std::forward<Self>(self).left.unwrap();
    } else {
      return compat::forward_like<Self>(std::forward<Self>(self).left);
    }
  }
};
}  // namespace slo::impl