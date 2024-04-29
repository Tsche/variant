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
template <typename... Ts>
class Variant;

template <std::size_t, typename>
struct variant_alternative;

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, Variant<Ts...>> {
  using type = util::pack::get<Idx, Variant<Ts...>>;
};

template <std::size_t Idx, typename... Ts>
struct variant_alternative<Idx, Variant<Ts...> const> {
  using type = util::pack::get<Idx, Variant<Ts...>> const;
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

template <typename F, typename Variant>
constexpr decltype(auto) visit(F&& visitor, Variant&& variant) {
#if defined(_MSC_VER)
// TODO macro solution
#else
  return impl::visit(std::forward<Variant>(variant), std::forward<F>(visitor), util::to_index_sequence<Variant>{});
#endif
}

template <typename... Ts>
class Variant {
  using storage_type = impl::Storage<HAS_IS_WITHIN_LIFETIME, Ts...>;

public:
  storage_type storage;
  static constexpr auto npos = storage_type::npos;

  template <typename T>
  explicit Variant(T&& obj)
    requires(std::same_as<std::remove_reference_t<T>, Ts> || ...)
      : storage{std::in_place_index<impl::type_index<std::remove_reference_t<T>, Ts...>>, std::forward<T>(obj)} {}

  template <std::size_t Idx, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args) : storage{idx, std::forward<Args>(args)...} {}

  template <typename T, typename... Args>
  explicit Variant(std::in_place_type_t<T>, Args&&... args)
      : storage{std::in_place_index<impl::type_index<T, Ts...>>, std::forward<Args>(args)...} {}

  constexpr Variant()
    requires(std::is_default_constructible_v<variant_alternative_t<0, Variant>>)
      : Variant(std::in_place_index<0>) {}

  constexpr Variant(Variant const&)  = default;
  constexpr Variant(Variant&&)       = default;
  Variant& operator=(Variant const&) = default;
  Variant& operator=(Variant&&)      = default;

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
};

}  // namespace slo