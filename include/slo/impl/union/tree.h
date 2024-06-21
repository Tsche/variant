#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>
#include <slo/util/compat.h>
#include <slo/impl/concepts.h>
#include <slo/util/list.h>

namespace slo::impl {

namespace detail {
template <bool Trivial, typename... Ts>
union TreeUnion;

template <bool Trivial>
union TreeUnion<Trivial>{};

template <bool Trivial, typename... Ts, typename... Us>
union TreeUnion<Trivial, TreeUnion<Trivial, Ts...>, TreeUnion<Trivial, Us...>> {
  constexpr static std::size_t size = TreeUnion<Trivial, Ts...>::size + TreeUnion<Trivial, Us...>::size;
  TreeUnion<Trivial, Ts...> left;
  TreeUnion<Trivial, Us...> right;

  constexpr TreeUnion(TreeUnion const&)                = default;
  constexpr TreeUnion(TreeUnion&&) noexcept            = default;
  constexpr TreeUnion& operator=(TreeUnion const&)     = default;
  constexpr TreeUnion& operator=(TreeUnion&&) noexcept = default;
  constexpr ~TreeUnion()                               = default;
  constexpr ~TreeUnion() requires(!Trivial) {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx> idx, Args&&... args)
    requires(Idx < TreeUnion<Trivial, Ts...>::size)
      : left{idx, std::forward<Args>(args)...} {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx>, Args&&... args)
    requires(Idx >= TreeUnion<Trivial, Ts...>::size)
      : right{std::in_place_index<Idx - TreeUnion<Trivial, Ts...>::size>, std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const {
    if (compat::is_within_lifetime(&left)) {
      return left.index();
    }
    return right.index();
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (Idx < TreeUnion<Trivial, Ts...>::size) {
      return std::forward<Self>(self).left.template get<Idx>();
    } else {
      return std::forward<Self>(self).right.template get<Idx - TreeUnion<Trivial, Ts...>::size>();
    }
  }
};

template <bool Trivial, typename... Ts, typename U>
union TreeUnion<Trivial, TreeUnion<Trivial, Ts...>, U> {
  constexpr static std::size_t size = 1 + TreeUnion<Trivial, Ts...>::size;
  TreeUnion<Trivial, Ts...> left;
  U right;

  constexpr TreeUnion(TreeUnion const&)                = default;
  constexpr TreeUnion(TreeUnion&&) noexcept            = default;
  constexpr TreeUnion& operator=(TreeUnion const&)     = default;
  constexpr TreeUnion& operator=(TreeUnion&&) noexcept = default;
  constexpr ~TreeUnion()                               = default;
  constexpr ~TreeUnion() requires(!Trivial) {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx> idx, Args&&... args)
    requires(Idx < TreeUnion<Trivial, Ts...>::size)
      : left{idx, std::forward<Args>(args)...} {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<Idx>, Args&&... args)
    requires(Idx == TreeUnion<Trivial, Ts...>::size)
      : right{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const {
    if (compat::is_within_lifetime(&right.tag)) {
      return right.tag;
    }
    return left.index();
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    if constexpr (Idx < TreeUnion<Trivial, Ts...>::size) {
      return std::forward<Self>(self).left.template get<Idx>();
    } else {
      static_assert(Idx == TreeUnion<Trivial, Ts...>::size, "Index out of bounds");
      if constexpr (has_unwrap<U>) {
        return std::forward<Self>(self).right.unwrap();
      } else {
        return compat::forward_like<Self>(std::forward<Self>(self).right);
      }
    }
  }
};

template <bool Trivial, typename T, typename U>
union TreeUnion<Trivial, T, U> {
  T left;
  U right;

  constexpr static std::size_t size = 2;

  constexpr TreeUnion(TreeUnion const&)                = default;
  constexpr TreeUnion(TreeUnion&&) noexcept            = default;
  constexpr TreeUnion& operator=(TreeUnion const&)     = default;
  constexpr TreeUnion& operator=(TreeUnion&&) noexcept = default;
  constexpr ~TreeUnion()                               = default;
  constexpr ~TreeUnion() requires(!Trivial) {}

  template <typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<0>, Args&&... args) : left{std::forward<Args>(args)...} {}

  template <typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<1>, Args&&... args) : right{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const {
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

template <bool Trivial, typename T>
union TreeUnion<Trivial, T> {
  T left;
  constexpr static std::size_t size = 1;

  constexpr TreeUnion(TreeUnion const&)                = default;
  constexpr TreeUnion(TreeUnion&&) noexcept            = default;
  constexpr TreeUnion& operator=(TreeUnion const&)     = default;
  constexpr TreeUnion& operator=(TreeUnion&&) noexcept = default;
  constexpr ~TreeUnion()                               = default;
  constexpr ~TreeUnion() requires(!Trivial) {}

  template <typename... Args>
  constexpr explicit TreeUnion(std::in_place_index_t<0>, Args&&... args) : left{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const {
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

template <bool, typename, typename>
struct ToCBTImpl;

template <bool Trivial>
struct ToCBTImpl<Trivial, util::TypeList<>, util::TypeList<>> {
  using type = TreeUnion<Trivial>;
};


template <bool Trivial, typename T>
struct ToCBTImpl<Trivial, util::TypeList<T>, util::TypeList<>> {
  using type = TreeUnion<Trivial, T>;
};

template <bool Trivial, typename T, typename U>
struct ToCBTImpl<Trivial, util::TypeList<>, util::TypeList<TreeUnion<Trivial, T, U>>> {
  using type = TreeUnion<Trivial, T, U>;
};

template <bool Trivial, typename... Out>
struct ToCBTImpl<Trivial, util::TypeList<>, util::TypeList<Out...>> {
  using type = ToCBTImpl<Trivial, util::TypeList<Out...>, util::TypeList<>>::type;
};

template <bool Trivial, typename T, typename... Out>
struct ToCBTImpl<Trivial, util::TypeList<T>, util::TypeList<Out...>> {
  using type = ToCBTImpl<Trivial, util::TypeList<Out..., T>, util::TypeList<>>::type;
};

template <bool Trivial, typename T1, typename T2, typename... In, typename... Out>
struct ToCBTImpl<Trivial, util::TypeList<T1, T2, In...>, util::TypeList<Out...>> {
  using type = ToCBTImpl<Trivial, util::TypeList<In...>, util::TypeList<Out..., TreeUnion<Trivial, T1, T2>>>::type;
};
}  // namespace detail

template <bool Trivial, typename... Ts>
using TreeUnion = typename detail::ToCBTImpl<Trivial, util::TypeList<Ts...>, util::TypeList<>>::type;

}  // namespace slo::impl