#pragma once
#include <cstddef>
#include <concepts>
#include <initializer_list>
#include <type_traits>
#include <utility>

#include "impl/feature.h"
#include "impl/concepts.h"
#include "impl/storage/inverted.h"
#include "impl/storage/normal.h"
#include "impl/storage/member_ptr.h"

#include "impl/visit/fptr_array.h"
#include "impl/visit/visit.h"
#if USING(SLO_MACRO_VISIT)
#  include "impl/visit/macro.h"
#else
#  include "impl/visit/variadic.h"
#endif

#include "util/compat.h"
#include "util/list.h"

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

template <typename F, typename... Vs>
constexpr decltype(auto) visit(F&& visitor, Vs&&... variants) {
  if constexpr (sizeof...(Vs) == 0) {
    return std::forward<F>(visitor)();
  } else {
    constexpr std::size_t max_index = impl::VisitImpl<Vs...>::max_index;
    constexpr int strategy          = max_index > 256          ? -1
                                      : USING(SLO_MACRO_VISIT) ? max_index <= 4    ? 1
                                                                 : max_index <= 16 ? 2
                                                                 : max_index <= 64 ? 3
                                                                                   : 4
                                                               : 0;
    return slo::impl::VisitStrategy<strategy>::visit(std::forward<F>(visitor), std::forward<Vs>(variants)...);
  }
}

inline constexpr std::size_t variant_npos = -1ULL;

namespace impl {
template <typename Source, typename Dest>
concept allowed_conversion = requires(Source obj) { std::type_identity_t<Dest[]>{std::move(obj)}; };

template <std::size_t Idx, typename T>
struct Build_FUN {
  template <allowed_conversion<T> U>
  auto operator()(T, U&&) -> std::integral_constant<std::size_t, Idx>;
};

template <typename V, typename = std::make_index_sequence<V::size>>
struct Build_FUNs;

template <template <typename...> class V, typename... Ts, std::size_t... Idx>
struct Build_FUNs<V<Ts...>, std::index_sequence<Idx...>> : Build_FUN<Idx, Ts>... {
  using Build_FUN<Idx, Ts>::operator()...;
};

template <typename T, typename V>
inline constexpr auto selected_index = variant_npos;

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

template <has_alternatives Storage>
class Variant {
public:
  using alternatives         = Storage::alternatives;
  static constexpr auto npos = Storage::npos;
  Storage storage;

  static_assert(alternatives::size > 0, "Variant must have at least one alternative");
  static_assert(!alternatives::template any<std::is_reference>, "Variant must not have reference alternatives");
  static_assert(!alternatives::template any<std::is_void>, "Variant must not have void alternatives");

  // default constructor, only if alternative #0 is default constructible
  constexpr Variant() noexcept(
      std::is_nothrow_default_constructible_v<variant_alternative_t<0, Variant>>)  // [variant.ctor]/5
    requires(std::is_default_constructible_v<variant_alternative_t<0, Variant>>)   // [variant.ctor]/2
      : Variant(std::in_place_index<0>) {}                                         // [variant.ctor]/3

  constexpr Variant(Variant const& other) = default;
  constexpr Variant(Variant const& other) noexcept(
      alternatives::template all<std::is_nothrow_copy_constructible>)  // [variant.ctor]/8
    requires(!std::is_trivially_destructible_v<Storage> &&
             alternatives::template all<std::is_copy_constructible>)  // [variant.ctor]/9
  {
    slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
  }

  constexpr Variant(Variant&& other) = default;
  constexpr Variant(Variant&& other) noexcept(
      alternatives::template all<std::is_nothrow_move_constructible>)  // [variant.ctor]/12
    requires(!std::is_trivially_destructible_v<Storage>                // [variant.ctor]/13
             && alternatives::template all<std::is_move_constructible>)
  {
    slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::forward<T>(obj)); }, std::move(other));
  }

  // converting constructor
  template <typename T>
    requires(alternatives::size != 0                              // [variant.ctor]/15.1
             && !std::same_as<std::remove_cvref_t<T>, Variant>    // [variant.ctor]/15.2
             && !is_in_place<std::remove_cvref_t<T>>              // [variant.ctor]/15.3
             && selected_index<T, alternatives> != variant_npos)  // [variant.ctor]/15.4, [variant.ctor]/15.5
  constexpr explicit Variant(T&& obj) noexcept(std::is_nothrow_constructible_v<  // [variant.ctor]/18
                                               typename util::type_at<selected_index<T, alternatives>, alternatives>,
                                               T>)
      : storage(std::in_place_index<selected_index<T, alternatives>>, std::forward<T>(obj)) {}

  // in place constructors
  template <typename T, typename... Args>
  explicit Variant(std::in_place_type_t<T>,
                   Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)  // [variant.ctor]/23
      : storage(std::in_place_index<util::index_of<T, alternatives>>, std::forward<Args>(args)...) {}

  template <typename T, typename U, typename... Args>
  explicit Variant(std::in_place_type_t<T>, std::initializer_list<U> init_list, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)  // [variant.ctor]/28
      : storage(std::in_place_index<util::index_of<T, alternatives>>, init_list, std::forward<Args>(args)...) {}

  template <std::size_t Idx, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<util::type_at<Idx, alternatives>, Args...>)  // [variant.ctor]/33
      : storage(idx, std::forward<Args>(args)...) {}

  template <std::size_t Idx, typename U, typename... Args>
  explicit Variant(std::in_place_index_t<Idx> idx,
                   std::initializer_list<U> init_list,
                   Args&&... args) noexcept(std::is_nothrow_constructible_v<util::type_at<Idx, alternatives>,
                                                                            std::initializer_list<U>&,
                                                                            Args...>)  // not standardized
      : storage(idx, init_list, std::forward<Args>(args)...) {}

  // TODO inplace + initializer list

  constexpr Variant& operator=(Variant const& other) = default;
  constexpr Variant& operator=(Variant&& other)      = default;

  Variant& operator=(Variant const& other)
    requires(!std::is_trivially_destructible_v<Storage> && alternatives::template all<std::is_copy_assignable>)
  {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
    }
    return *this;
  }

  Variant& operator=(Variant&& other) noexcept
    requires(!std::is_trivially_destructible_v<Storage> && alternatives::template all<std::is_move_assignable>)
  {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::forward<T>(obj)); }, std::move(other));
    }
    return *this;
  }

  constexpr ~Variant() = default;

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args) {
    storage.template emplace<util::index_of<T, alternatives>>(std::forward<Args>(args)...);
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
    return slo::get<util::index_of<T, alternatives>>(std::forward<Self>(self).storage);
  }

  template <typename Self, typename V>
  constexpr decltype(auto) visit(this Self&& self, V&& visitor) {
    return slo::visit(std::forward<V>(visitor), std::forward<Self>(self));
  }

  [[nodiscard]] constexpr bool valueless_by_exception() const noexcept { return storage.index() == npos; }
  [[nodiscard]] constexpr std::size_t index() const noexcept {
    if (auto idx = storage.index(); idx != npos) {
      return idx;
    }
    return variant_npos;
  }
};
}  // namespace impl

template <std::size_t Idx, impl::has_alternatives T>
struct variant_alternative<Idx, T> {
  static_assert(Idx < T::alternatives::size, "variant_alternative index out of range");
  using type = util::type_at<Idx, typename T::alternatives>;
};

template <std::size_t Idx, impl::has_alternatives T>
struct variant_alternative<Idx, T const> {
  static_assert(Idx < T::alternatives::size, "variant_alternative index out of range");
  using type = util::type_at<Idx, typename T::alternatives> const;
};

template <std::size_t Idx, impl::has_alternatives T>
struct variant_alternative<Idx, T volatile> {
  static_assert(Idx < T::alternatives::size, "variant_alternative index out of range");
  using type = util::type_at<Idx, typename T::alternatives> volatile;
};

template <std::size_t Idx, impl::has_alternatives T>
struct variant_alternative<Idx, T const volatile> {
  static_assert(Idx < T::alternatives::size, "variant_alternative index out of range");
  using type = util::type_at<Idx, typename T::alternatives> const volatile;
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
    impl::Variant<typename impl::StorageChoice<(HAS_IS_WITHIN_LIFETIME && (std::is_standard_layout_v<Ts> && ...)),
                                               impl::InvertedStorage,
                                               impl::Storage>::template type<Ts...>>;

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