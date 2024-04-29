#pragma once
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>

#include "impl/storage.h"
#include "util/compat.h"
#include "util/list.h"
#include "util/pack.h"

namespace slo {
namespace impl {
template <typename, typename...>
class Variant;
}

template <std::size_t, typename>
struct variant_alternative;

template <std::size_t Idx, typename T, typename... Ts>
struct variant_alternative<Idx, impl::Variant<T, Ts...>> {
  using type = util::pack::get<Idx, impl::Variant<Ts...>>;
};

template <std::size_t Idx, typename T, typename... Ts>
struct variant_alternative<Idx, impl::Variant<T, Ts...> const> {
  using type = util::pack::get<Idx, impl::Variant<Ts...>> const;
};

template <std::size_t Idx, typename V>
using variant_alternative_t = typename variant_alternative<Idx, std::remove_reference_t<V>>::type;

template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<Idx>();
}

template <typename T, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<T>();
}

namespace impl {
template <typename Variant, typename F, std::size_t... Is>
constexpr decltype(auto) visit(Variant&& variant, F visitor, std::index_sequence<Is...>) {
  using return_type = std::common_type_t<decltype(visitor(slo::get<Is>(std::forward<Variant>(variant))))...>;
  if constexpr (std::same_as<return_type, void>) {
    (void)((variant.index() == Is ? visitor(slo::get<Is>(std::forward<Variant>(variant))), true : false) || ...);
  } else {
    union {
      char dummy;
      return_type value;
    } ret{};

    bool const success =
        ((variant.index() == Is ? std::construct_at(&ret.value, visitor(slo::get<Is>(std::forward<Variant>(variant)))),
          true                  : false) ||
         ...);

    if (success) {
      return ret.value;
    }
    std::unreachable();
  }
}
}  // namespace impl

template <typename F, typename V>
constexpr decltype(auto) visit(F&& visitor, V&& variant) {
#if defined(_MSC_VER)
// TODO macro solution
#else
  return impl::visit(std::forward<V>(variant), std::forward<F>(visitor),
                     std::make_index_sequence<std::remove_reference_t<V>::size>{});
#endif
}
namespace impl {
template <typename storage_type, typename... Ts>
class Variant {
public:
  storage_type storage;
  static constexpr auto npos = storage_type::npos;
  static constexpr auto size = sizeof...(Ts);

  template <typename T>
  explicit Variant(T&& obj)
    requires(std::same_as<std::remove_reference_t<T>, Ts> || ...)
      : storage(std::in_place_index<impl::type_index<std::remove_reference_t<T>, Ts...>>, std::forward<T>(obj)) {}

  template <std::size_t Idx, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args) : storage(idx, std::forward<Args>(args)...) {}

  template <typename T, typename... Args>
  explicit Variant(std::in_place_type_t<T>, Args&&... args)
      : storage(std::in_place_index<impl::type_index<T, Ts...>>, std::forward<Args>(args)...) {}

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
    storage.template emplace<impl::type_index<T, Ts...>>(std::forward<Args>(args)...);
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
    return slo::get<impl::type_index<T, Ts...>>(std::forward<Self>(self).storage);
  }

  template <typename Self, typename V>
  constexpr decltype(auto) visit(this Self&& self, V&& visitor) {
    return slo::visit(std::forward<V>(visitor), std::forward<Self>(self));
  }

  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return storage.index() == npos; }
  [[nodiscard]] constexpr std::size_t index() const noexcept { return storage.index(); }
};
}  // namespace impl
// using storage_type = impl::Storage<HAS_IS_WITHIN_LIFETIME, Ts...>;

template <typename... Ts>
using Variant = impl::Variant<impl::Storage<false, Ts...>, Ts...>;

template <typename... Ts>
using Union = impl::Variant<impl::Storage<true, Ts...>, Ts...>;

template <typename... Ts>
using variant = impl::Variant<impl::Storage<HAS_IS_WITHIN_LIFETIME, Ts...>, Ts...>;

}  // namespace slo