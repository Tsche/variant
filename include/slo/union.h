#pragma once
#include <cstddef>
#include <memory>
#include <utility>

#include "impl/visit.h"
#include "impl/common.h"
#include "impl/wrapper.h"

#include "impl/union/tree.h"
#include "impl/union/recursive.h"

namespace slo {
template <typename... Ts>
union Union {
  constexpr static unsigned char npos = static_cast<unsigned char>(~0U);

  template <std::size_t... Is>
  static constexpr auto generate_union(std::index_sequence<Is...>)
      -> impl::RecursiveUnion<impl::Wrapper<static_cast<unsigned char>(Is), Ts>...>;

  template <std::size_t... Is>
    requires(sizeof...(Ts) >= 42)
  static constexpr auto generate_union(std::index_sequence<Is...>) 
    -> util::to_cbt<impl::TreeUnion, impl::Wrapper<static_cast<unsigned char>(Is), Ts>...>;

  using union_type = decltype(generate_union(std::index_sequence_for<Ts...>()));
  using dummy_type = impl::Wrapper<npos, nullopt>;

  dummy_type dummy;
  struct Storage {
    union_type value;
    template <typename... Args>
    constexpr explicit Storage(Args&&... args) : value(std::forward<Args>(args)...) {}
    constexpr ~Storage() = default;
  } storage;

  template <std::size_t Idx, typename... Args>
  constexpr explicit Union(std::in_place_index_t<Idx> idx, Args&&... args)
      : storage{idx, std::forward<Args>(args)...} {}

  template <typename T>
  constexpr explicit Union(T&& item)
    requires(!std::same_as<Union, std::remove_cvref_t<T>> && (std::same_as<std::remove_cvref_t<T>, Ts> || ...))
      : storage(std::in_place_index<impl::type_index<T, Ts...>>, std::forward<T>(item)) {}

  constexpr Union() 
    requires (std::is_default_constructible_v<variant_alternative_t<0, Union>>)
    : Union(std::in_place_index<0>) {}

  constexpr Union(Union const& other) {
    visit([this](auto const& obj) { this->emplace(obj); }, other);
  }

  constexpr Union(Union&& other) {
    visit([this]<typename T>(T&& obj) { this->emplace(std::move(obj)); }, std::move(other));
  }

  constexpr Union& operator=(Union const& other) {
    if (this != std::addressof(other)) {
      visit([this](auto const& obj) { this->emplace(obj); }, other);
    }
    return *this;
  }

  constexpr Union& operator=(Union&& other) {
    if (this != std::addressof(other)) {
      visit([this]<typename T>(T&& obj) { this->emplace(std::move(obj)); }, std::move(other));
    }
    return *this;
  }

  constexpr ~Union() { reset(); }

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
    std::construct_at(&storage, std::in_place_index<Idx>, std::forward<Args>(args)...);
  }

  template <typename T>
  constexpr void emplace(T&& obj) {
    reset();
    std::construct_at(&storage, std::in_place_index<impl::type_index<T, Ts...>>, std::forward<T>(obj));
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.value.template get<Idx>();
  }

  template <typename T, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).template get<impl::type_index<T, Ts...>>();
  }

  constexpr bool valueless_by_exception() const noexcept{
    return index() == npos;
  }
};
}  // namespace slo