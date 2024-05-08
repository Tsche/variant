#pragma once
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>

#include "impl/concepts.h"
#include "impl/storage.h"

#if defined(_MSC_VER)
#include "impl/visit/variadic.h"
#else
#include "impl/visit/macro.h"
#endif

#include "util/compat.h"
#include "util/list.h"
#include "util/pack.h"

namespace slo {

template <std::size_t, typename>
struct variant_alternative;

template <typename T>
struct variant_size;

template <std::size_t Idx, typename V>
using variant_alternative_t = typename variant_alternative<Idx, std::remove_reference_t<V>>::type;

template <typename T>
inline constexpr std::size_t variant_size_v = slo::variant_size<T>::value;

template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<Idx>();
}

template <typename T, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<T>();
}

template <typename F, typename V>
constexpr decltype(auto) visit(F&& visitor, V&& variant) {
#if !defined(_MSC_VER)
  return impl::macro::visit(std::forward<F>(visitor), std::forward<V>(variant));
#else
  return impl::variadic::visit(std::forward<F>(visitor), std::forward<V>(variant),
                     std::make_index_sequence<variant_size_v<std::remove_reference_t<V>>>{});
#endif
}

namespace impl {
template <template <typename...> class Storage, typename... Ts>
class Variant {
  using storage_type = Storage<Ts...>;
public:
  storage_type storage;
  static constexpr auto npos = storage_type::npos;
  static constexpr auto size = sizeof...(Ts);

  template <typename T>
  explicit Variant(T&& obj)
    requires(std::same_as<std::remove_reference_t<T>, Ts> || ...)
      : storage(std::in_place_index<type_index<std::remove_reference_t<T>, Ts...>>, std::forward<T>(obj)) {}

  template <std::size_t Idx, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args) : storage(idx, std::forward<Args>(args)...) {}

  template <typename T, typename... Args>
  explicit Variant(std::in_place_type_t<T>, Args&&... args)
      : storage(std::in_place_index<type_index<T, Ts...>>, std::forward<Args>(args)...) {}

  constexpr Variant()
    requires(std::is_default_constructible_v<variant_alternative_t<0, Variant>>)
      : Variant(std::in_place_index<0>) {}

  constexpr Variant(Variant const& other) {
    slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
  }
  constexpr Variant(Variant&& other) {
    slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::move(obj)); }, std::move(other));
  }
  Variant& operator=(Variant const& other) {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
    }
    return *this;
  }
  Variant& operator=(Variant&& other) {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::move(obj)); }, std::move(other));
    }
    return *this;
  }

  constexpr ~Variant() { storage.reset(); }

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args) {
    storage.template emplace<type_index<T, Ts...>>(std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    storage.template emplace<Idx>(std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return slo::get<Idx>(std::forward<Self>(self).storage);
  }

  template <typename T, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return slo::get<type_index<T, Ts...>>(std::forward<Self>(self).storage);
  }

  template <typename Self, typename V>
  constexpr decltype(auto) visit(this Self&& self, V&& visitor) {
    return slo::visit(std::forward<V>(visitor), std::forward<Self>(self));
  }

  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return storage.index() == npos; }
  [[nodiscard]] constexpr std::size_t index() const noexcept { return storage.index(); }
};
}  // namespace impl

template <std::size_t Idx, template <typename...> class T, typename... Ts>
struct variant_alternative<Idx, impl::Variant<T, Ts...>> {
  using type = util::pack::get<Idx, util::TypeList<Ts...>>;
};

template <std::size_t Idx, template <typename...> class T, typename... Ts>
struct variant_alternative<Idx, impl::Variant<T, Ts...> const> {
  using type = util::pack::get<Idx, util::TypeList<Ts...>> const;
};

template <template <typename...> class T, typename... Ts>
struct variant_size<impl::Variant<T, Ts...>> {
  static constexpr std::size_t value = sizeof...(Ts);
};

template <template <typename...> class T, typename... Ts>
struct variant_size<impl::Variant<T, Ts...> const> {
  static constexpr std::size_t value = sizeof...(Ts);
};

template <typename... Ts>
using RegularVariant = impl::Variant<impl::Storage, Ts...>;

template <typename... Ts>
using InvertedVariant = impl::Variant<impl::InvertedStorage, Ts...>;

template <typename... Ts>
using Variant = impl::Variant<impl::StorageChoice<HAS_IS_WITHIN_LIFETIME, impl::InvertedStorage, impl::Storage>::type, Ts...>;

template <typename... Ts>
using variant = Variant<Ts...>;

}  // namespace slo