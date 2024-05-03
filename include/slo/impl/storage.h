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
struct nullopt {};

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

template <typename... Ts>
using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;

template <typename... Ts>
class Storage {
  // discriminated union
public:
  using index_type                 = index_type<Ts...>;
  constexpr static index_type npos = static_cast<index_type>(~0U);

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
  constexpr Storage() : dummy(), tag(npos) {}
  constexpr ~Storage() {}

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  constexpr void reset() {
    if (tag != npos) {
      slo::visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
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
union InvertedStorage {
  // inverted variant
  using index_type                 = index_type<Ts...>;
  constexpr static index_type npos = static_cast<index_type>(~0U);

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
  } storage;

public:
  template <typename... Args>
  constexpr explicit InvertedStorage(Args&&... args) : storage{union_type(std::forward<Args>(args)...)} {}
  constexpr InvertedStorage() : dummy() {}
  constexpr ~InvertedStorage() {}

  [[nodiscard]] constexpr std::size_t index() const {
    if consteval {
      if (!compat::is_within_lifetime(&dummy.tag)) {
        return storage.value.index();
      }
    }
    return dummy.index();
  }

  constexpr void reset() {
    slo::visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
    std::construct_at(&dummy);
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&storage.value, std::in_place_index<Idx>, std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.value.template get<Idx>();
  }
};

template <bool, template <typename...> class T, template <typename...> class F>
struct StorageChoice;

template <template <typename...> class T, template <typename...> class F>
struct StorageChoice<true, T, F> {
  template <typename... Ts>
  using type = T<Ts...>;
};

template <template <typename...> class T, template <typename...> class F>
struct StorageChoice<false, T, F> {
  template <typename... Ts>
  using type = F<Ts...>;
};
}  // namespace impl

template <typename T>
struct variant_size;

template <typename... Ts>
struct variant_size<impl::Storage<Ts...>> {
  static constexpr std::size_t value = sizeof...(Ts);
};

template <typename... Ts>
struct variant_size<impl::Storage<Ts...> const> {
  static constexpr std::size_t value = sizeof...(Ts);
};

template <typename... Ts>
struct variant_size<impl::InvertedStorage<Ts...>> {
  static constexpr std::size_t value = sizeof...(Ts);
};

template <typename... Ts>
struct variant_size<impl::InvertedStorage<Ts...> const> {
  static constexpr std::size_t value = sizeof...(Ts);
};
}  // namespace slo