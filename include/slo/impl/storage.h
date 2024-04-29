#pragma once
#include <memory>
#include <utility>
#include <cstddef>
#include <concepts>
#include <type_traits>

#include <slo/util/list.h>

#include "wrapper.h"
#include "union/recursive.h"
#include "union/tree.h"

namespace slo {
struct nullopt{};

template <typename F, typename Variant>
constexpr decltype(auto) visit(F&& visitor, Variant&& variant);

namespace impl {
struct error_type {};

template <typename T, typename... Ts>
consteval std::size_t get_type_index() {
  std::size_t index = 0;
  (void)((!std::same_as<T, Ts> ? ++index, false : true) || ...);
  return index;
}

template <typename T, typename... Ts>
constexpr inline std::size_t type_index = get_type_index<std::remove_cvref_t<T>, Ts...>();

template <typename T>
concept has_get = requires(T obj) { obj.template get<0>(); };

template <typename... Ts>
using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;

template <bool, typename...>
class Storage;

template <typename... Ts>
class Storage<false, Ts...> {
  // discriminated union
public:
  using index_type                  = index_type<Ts...>;
  constexpr static index_type npos  = static_cast<index_type>(~0U);
  constexpr static std::size_t size = sizeof...(Ts);

private:
  template <typename... Us>
  static consteval auto generate_union() -> RecursiveUnion<Us...>;

  template <typename... Us>
    requires(sizeof...(Us) >= 42)
  static consteval auto generate_union() -> util::to_cbt<TreeUnion, Ts...>;

  using union_type = decltype(generate_union<Ts...>());
  union {
    error_type dummy;
    union_type value;
  };

  index_type tag{npos};

public:
  template <std::size_t Idx, typename... Args>
  constexpr explicit Storage(std::in_place_index_t<Idx> idx, Args&&... args) 
    : value(idx, std::forward<Args>(args)...)
    , tag(Idx) {}
  constexpr Storage() : dummy(), tag(npos){}
  constexpr ~Storage(){}

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  constexpr void reset() {
    if (tag != npos) {
      visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
      tag = npos;
    }
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&value, std::in_place_index<Idx>, std::forward<Args>(args)...);
    tag = Idx;
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).value.template get<Idx>();
  }
};

template <typename... Ts>
class Storage<true, Ts...> {
  // inverted variant
public:
  using index_type                  = index_type<Ts...>;
  constexpr static index_type npos  = static_cast<index_type>(~0U);
  constexpr static std::size_t size = sizeof...(Ts);

private:
  union Union {
    template <typename... Args>
    constexpr explicit Union(Args&&... args) : container(std::forward<Args>(args)...) {}
    constexpr Union() : dummy() {}
    constexpr ~Union() {}

    template <std::size_t... Is>
    static constexpr auto generate_union(std::index_sequence<Is...>)
        -> RecursiveUnion<Wrapper<static_cast<index_type>(Is), Ts>...>;

    template <std::size_t... Is>
      requires(sizeof...(Ts) >= 42)
    static constexpr auto generate_union(std::index_sequence<Is...>)
        -> util::to_cbt<TreeUnion, Wrapper<static_cast<index_type>(Is), Ts>...>;

    using union_type = decltype(generate_union(std::index_sequence_for<Ts...>()));

    Wrapper<npos, error_type> dummy;
    struct Container {
      union_type value;
      template <typename... Args>
      constexpr explicit Container(Args&&... args) : value(std::forward<Args>(args)...) {}
      constexpr ~Container() = default;
    } container;
  } storage;

public:
  template <typename... Args>
  constexpr explicit Storage(Args&&... args) : storage(std::forward<Args>(args)...) {}
  constexpr Storage() : storage() {}
  constexpr ~Storage(){}

  [[nodiscard]] constexpr std::size_t index() const {
    if consteval {
      if (!compat::is_within_lifetime(&storage.dummy.tag)) {
        return storage.container.value.index();
      }
    }
    return storage.dummy.index();
  }

  constexpr void reset() {
    visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
    std::construct_at(&storage.dummy);
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&storage.container, std::in_place_index<Idx>, std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.container.value.template get<Idx>();
  }
};
}  // namespace impl
}  // namespace slo