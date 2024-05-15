#pragma once
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>

#include "impl/concepts.h"
#include "impl/storage/inverted.h"
#include "impl/storage/normal.h"
#include "impl/storage/member_ptr.h"

#if defined(_MSC_VER)
#  include "impl/visit/variadic.h"
#else
#  include "impl/visit/macro.h"
#endif

#include "util/compat.h"
#include "util/list.h"
#include "util/pack.h"

namespace slo {

struct nullopt {};

template <std::size_t, typename>
struct variant_alternative;

template <typename>
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
template <typename Source, typename Dest>
concept allowed_conversion = requires { std::type_identity_t<Dest[]>{std::declval<Source>()}; };

template <std::size_t Idx, typename T>
struct Build_FUN {
  template <allowed_conversion<T> U>
  auto operator()(T, U&&) -> std::integral_constant<std::size_t, Idx>;
};

template <typename V, typename = std::make_index_sequence<std::variant_size_v<V>>>
struct Build_FUNs;

template <template <typename...> class V, typename... Ts, std::size_t... Idx>
struct Build_FUNs<V<Ts...>, std::index_sequence<Idx...>> : Build_FUN<Idx, Ts>... {
  using Build_FUN<Idx, Ts>::operator()...;
};

template <typename T, typename V>
inline constexpr auto selected_index = std::variant_npos;

template <typename T, typename V>
  requires std::invocable<Build_FUNs<V>, T, T>
inline constexpr auto selected_index<T, V> = std::invoke_result_t<Build_FUNs<V>, T, T>::value;

template <typename>
struct in_place_tag : std::false_type {};

template <typename T>
struct in_place_tag<std::in_place_type_t<T>> : std::true_type {};
template <std::size_t V>
struct in_place_tag<std::in_place_index_t<V>> : std::true_type {};

template <typename T>
concept is_in_place = in_place_tag<T>::value;

template <bool, template <typename...> class T, template <typename...> class F>
struct StorageChoice;  // TODO find better name, move out of this file

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

template <has_alternatives T, typename = typename T::alternatives>
class Variant;

template <has_alternatives Storage, typename... Ts>
class Variant<Storage, util::TypeList<Ts...>> {
public:
  using alternatives = Storage::alternatives;
  Storage storage;
  static constexpr auto npos = Storage::npos;
  static constexpr auto size = sizeof...(Ts);

  // default constructor, only if alternative #0 is default constructible
  constexpr Variant()
    requires(std::is_default_constructible_v<variant_alternative_t<0, Variant>>)
      : Variant(std::in_place_index<0>) {}

  // in place constructors
  template <std::size_t Idx, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args) : storage(idx, std::forward<Args>(args)...) {}

  template <typename T, typename... Args>
  explicit Variant(std::in_place_type_t<T>, Args&&... args)
      : storage(std::in_place_index<alternatives::template get_index<T>>, std::forward<Args>(args)...) {}

  // TODO inplace + initializer list

  // converting constructor
  template <typename T>
    requires(sizeof...(Ts) != 0 && !std::same_as<std::remove_cvref_t<T>, Variant> &&
             !is_in_place<std::remove_cvref_t<T>> && selected_index<T, typename Storage::alternatives> != npos)
  constexpr explicit Variant(T&& obj)
      : storage(std::in_place_index<selected_index<T, Storage>>, std::forward<T>(obj)) {}

  constexpr Variant(Variant const& other)                = default;
  constexpr Variant(Variant&& other) noexcept            = default;
  constexpr Variant& operator=(Variant const& other)     = default;
  constexpr Variant& operator=(Variant&& other) noexcept = default;
  constexpr ~Variant()                                   = default;

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args) {
    storage.template emplace<alternatives::template get_index<T>>(std::forward<Args>(args)...);
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
    return slo::get<alternatives::template get_index<T>>(std::forward<Self>(self).storage);
  }

  template <typename Self, typename V>
  constexpr decltype(auto) visit(this Self&& self, V&& visitor) {
    return slo::visit(std::forward<V>(visitor), std::forward<Self>(self));
  }

  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return storage.index() == npos; }
  [[nodiscard]] constexpr std::size_t index() const noexcept { return storage.index(); }
};
}  // namespace impl

template <std::size_t Idx, impl::has_alternatives T>
struct variant_alternative<Idx, T> {
  using type = util::pack::get<Idx, typename T::alternatives>;
};

template <std::size_t Idx, impl::has_alternatives T>
struct variant_alternative<Idx, T const> {
  using type = util::pack::get<Idx, typename T::alternatives>;
};

template <impl::has_alternatives T>
struct variant_size<T> {
  static constexpr std::size_t value = T::alternatives::size;
};

template <impl::has_alternatives T>
struct variant_size<T const> {
  static constexpr std::size_t value = T::alternatives::size;
};

/**
 * @brief This implementation of variant 
 * 
 * @tparam Ts Alternative types
 */
template <typename... Ts>
using NormalVariant = impl::Variant<impl::Storage<Ts...>>;

/**
 * @brief This implementation of variant hides the index in the actual alternative types
 *        hence inverting the variant.
 * @warning Your compiler must support C++26 `std::is_within_lifetime` for this to work in
 *          constant evaluated context. If your compiler has an intrinsic that can be used to
 *          implement `is_within_lifetime`, it will be used.
 * 
 * @tparam Ts Alternative types
 */
template <typename... Ts>
using InvertedVariant = impl::Variant<impl::InvertedStorage<Ts...>>;


/**
 * @brief If the compiler supports `std::is_within_lifetime` or has an intrinsic
 *        that can be used to implement this, this type will use the inverted
 *        storage strategy to reduce padding as much as possible.
 * 
 * @tparam Ts Alternative types
 */
template <typename... Ts>
using Variant =
    impl::Variant<impl::StorageChoice<HAS_IS_WITHIN_LIFETIME, impl::InvertedStorage, impl::Storage>::type<Ts...>>;

/**
 * @brief Same as slo::Variant. Provided for compatibility with `std::variant`.
 * 
 * @tparam Ts Alternative types
 */
template <typename... Ts>
using variant = Variant<Ts...>;

/**
 * @brief This variant implementation does not generate the underlying union,
 *        instead it is templated over pointers to non-static data members
 *        of an existing complete union type.
 * @warning All template arguments must refer to pointers to non-static data members of the same union.
 * @tparam Ptrs Pointers to non-static data members of a union
 */
template <auto... Ptrs>
using Union = impl::Variant<impl::StorageProxy<Ptrs...>>;

template <typename... Ts, typename... Args>
Variant<Ts...> make_variant(Args&&... args) {
  return {std::forward<Args>(args)...};
}

template <auto... Ptrs, typename... Args>
Union<Ptrs...> make_variant(Args&&... args) {
  return {std::forward<Args>(args)...};
}

}  // namespace slo