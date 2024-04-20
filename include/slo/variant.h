#pragma once
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>

#include "impl/union/tree.h"
#include "impl/union/recursive.h"
#include "impl/common.h"
#include "util/list.h"
#include "impl/visit.h"

namespace slo {
template <typename... Ts>
class Variant {
public:
  using index_type = std::conditional_t<(sizeof...(Ts) >= 255), unsigned short, unsigned char>;
  static constexpr index_type npos = static_cast<index_type>(~0U);

private:
  template <typename... Us>
    static consteval auto generate_union() -> impl::RecursiveUnion<Us...>;

    template <typename... Us>
      requires(sizeof...(Us) >= 42)
    static consteval auto generate_union() -> util::to_cbt<impl::TreeUnion, Ts...>;
  using union_type = decltype(generate_union<Ts...>());
  union {
    nullopt dummy;
    union_type value;
  };

  index_type tag{npos};

public:
  template <typename T>
  explicit Variant(T&& obj)
    requires(std::same_as<std::remove_reference_t<T>, Ts> || ...)
      : value{std::in_place_index<impl::type_index<std::remove_reference_t<T>, Ts...>>, std::forward<T>(obj)}
      , tag{impl::type_index<std::remove_reference_t<T>, Ts...>} {}

  template <std::size_t Idx, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args)
      : value{idx, std::forward<Args>(args)...}
      , tag{Idx} {}

  template <typename T, typename... Args>
  explicit Variant(std::in_place_type_t<T>, Args&&... args)
      : value{std::in_place_index<impl::type_index<T, Ts...>>, std::forward<Args>(args)...}
      , tag{impl::type_index<T, Ts...>} {}

  constexpr Variant() 
    requires (std::is_default_constructible_v<variant_alternative_t<0, Variant>>)
    : Variant(std::in_place_index<0>) {}

  constexpr ~Variant() { reset(); }

  constexpr Variant(Variant const& other) {
    visit([this](auto const& obj) { this->emplace(obj); }, other);
  }

  constexpr Variant(Variant&& other) {
    visit([this]<typename T>(T&& obj) { this->emplace(std::move(obj)); }, std::move(other));
  }

  constexpr Variant& operator=(Variant const& other) {
    if (this != std::addressof(other)) {
      visit([this](auto const& obj) { this->emplace(obj); }, other);
    }
    return *this;
  }

  constexpr Variant& operator=(Variant&& other) {
    if (this != std::addressof(other)) {
      visit([this]<typename T>(T&& obj) { this->emplace(std::move(obj)); }, std::move(other));
    }
    return *this;
  }

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  constexpr void reset() {
    if (tag != npos){
      visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
      tag = npos;
    }
  }

  template <typename T>
  constexpr void emplace(T&& obj) {
    reset();
    constexpr auto idx = impl::type_index<T, Ts...>;
    std::construct_at(&value, std::in_place_index<idx>, std::forward<T>(obj));
    // the ctor didn't throw if this is reached, switch tag
    tag = idx;
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

  template <typename T, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).template get<impl::type_index<T, Ts...>>();
  }

  constexpr bool valueless_by_exception() const noexcept{
    return tag == npos;
  }
};

}  // namespace slo