#pragma once
#include <cstddef>
#include <concepts>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <functional>

#include "impl/feature.h"
#include "impl/concepts.h"
#include "impl/storage/inverted.h"
#include "impl/storage/normal.h"
#include "impl/storage/member_ptr.h"

#include "impl/visit/fptr_array.h"
#include "impl/visit/visit.h"
#include "slo/impl/union/recursive.h"
#include "slo/impl/union/tree.h"
#if USING(SLO_MACRO_VISIT)
#  include "impl/visit/macro.h"
#else
#  include "impl/visit/variadic.h"
#endif

#include "util/concepts.h"
#include "util/compat.h"
#include "util/list.h"
#include "util/utility.h"

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

template <template <typename...> class Storage, typename... Ts>
class Variant {
public:
  using alternatives         = Storage<Ts...>::alternatives;
  static constexpr auto npos = Storage<Ts...>::npos;
  Storage<Ts...> storage;

  static_assert(alternatives::size > 0, "Variant must contain at least one alternative");
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
    requires(!std::is_trivially_destructible_v<Storage<Ts...>> &&
             alternatives::template all<std::is_copy_constructible>)  // [variant.ctor]/9
  {
    slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
  }

  constexpr Variant(Variant&& other) = default;
  constexpr Variant(Variant&& other) noexcept(
      alternatives::template all<std::is_nothrow_move_constructible>)  // [variant.ctor]/12
    requires(!std::is_trivially_destructible_v<Storage<Ts...>>         // [variant.ctor]/13
             && alternatives::template all<std::is_move_constructible>)
  {
    slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::forward<T>(obj)); }, std::move(other));
  }

  // converting constructor
  template <typename T>
    requires(alternatives::size != 0                              // [variant.ctor]/15.1
             && !std::same_as<std::remove_cvref_t<T>, Variant>    // [variant.ctor]/15.2
             && !util::is_in_place<std::remove_cvref_t<T>>        // [variant.ctor]/15.3
             && selected_index<T, alternatives> != variant_npos)  // [variant.ctor]/15.4, [variant.ctor]/15.5
  constexpr explicit Variant(T&& obj) noexcept(std::is_nothrow_constructible_v<  // [variant.ctor]/18
                                               typename util::type_at<selected_index<T, alternatives>, alternatives>,
                                               T>)
      : storage(std::in_place_index<selected_index<T, alternatives>>, std::forward<T>(obj)) {}

  // in place constructors
  template <typename T, typename... Args>
  constexpr explicit Variant(std::in_place_type_t<T>,
                             Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)  // [variant.ctor]/23
      : storage(std::in_place_index<util::index_of<T, alternatives>>, std::forward<Args>(args)...) {}

  template <typename T, typename U, typename... Args>
  constexpr explicit Variant(std::in_place_type_t<T>, std::initializer_list<U> init_list, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)  // [variant.ctor]/28
      : storage(std::in_place_index<util::index_of<T, alternatives>>, init_list, std::forward<Args>(args)...) {}

  template <std::size_t Idx, typename... Args>
  constexpr explicit Variant(std::in_place_index_t<Idx> idx, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<util::type_at<Idx, alternatives>, Args...>)  // [variant.ctor]/33
      : storage(idx, std::forward<Args>(args)...) {}

  template <std::size_t Idx, typename U, typename... Args>
  constexpr explicit Variant(std::in_place_index_t<Idx> idx,
                             std::initializer_list<U> init_list,
                             Args&&... args) noexcept(std::is_nothrow_constructible_v<util::type_at<Idx, alternatives>,
                                                                                      std::initializer_list<U>&,
                                                                                      Args...>)  // noexcept not standardized
      : storage(idx, init_list, std::forward<Args>(args)...) {}

  // TODO inplace + initializer list

  constexpr Variant& operator=(Variant const& other) = default;
  constexpr Variant& operator=(Variant&& other)      = default;

  constexpr Variant& operator=(Variant const& other)
    requires(!std::is_trivially_destructible_v<Storage<Ts...>> && alternatives::template all<std::is_copy_assignable>)
  {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
    }
    return *this;
  }

  constexpr Variant& operator=(Variant&& other) noexcept
    requires(!std::is_trivially_destructible_v<Storage<Ts...>> && alternatives::template all<std::is_move_assignable>)
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
    return std::forward<Self>(self).storage.template get<Idx>();
  }

  template <typename T, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.template get<util::index_of<T, alternatives>>();
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

  void swap(Variant& other) {
    if (valueless_by_exception() && other.valueless_by_exception()) {
      // both variants valueless - do nothing
      return;
    }
    if (index() == other.index()){
      // same index, swap the held objects directly
      slo::visit([](auto& own_value, auto& other_value){
        using std::swap;
        swap(own_value, other_value);
      }, *this, other);
    }
#if __cpp_exceptions
#else
#endif
  }
};
}  // namespace impl

//? [variant.helper], variant helper classes

template <typename T>
struct variant_size;

template <typename T>
struct variant_size : std::integral_constant<std::size_t, std::remove_cv_t<T>::alternatives::size> {
  // Retrieve size from alternatives.
  // Note that this primary template cannot be undefined as the standard mandates.

  // Unfortunately constraining T causes this to error early
  static_assert(impl::has_alternatives<std::remove_cv_t<T>>,
                "variant_size must be used on slo::variant or its underlying storage.");
};

// Workaround for variant_size<slo::Variant<>> to compile
template <template <typename...> class Storage, typename... Ts>
struct variant_size<impl::Variant<Storage, Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <template <typename...> class Storage, typename... Ts>
struct variant_size<impl::Variant<Storage, Ts...> const> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <template <typename...> class Storage, typename... Ts>
struct variant_size<impl::Variant<Storage, Ts...> volatile> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <template <typename...> class Storage, typename... Ts>
struct variant_size<impl::Variant<Storage, Ts...> const volatile>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

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

//? [variant.get], value access

template <std::size_t Idx, template <typename...> class Storage, typename... Types>
constexpr bool holds_alternative(impl::Variant<Storage, Types...> const& obj) noexcept {
  // this one isn't standardized, but it's useful anyway
  return obj.index() == Idx;
}

template <class T, template <typename...> class Storage, typename... Types>
constexpr bool holds_alternative(impl::Variant<Storage, Types...> const& obj) noexcept {
  return holds_alternative<util::index_of<T, typename impl::Variant<Storage, Types...>::alternatives>>(obj);
}

template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get_unchecked(V&& variant_) noexcept {
  return std::forward<V>(variant_).template get<Idx>();
}

template <typename T, impl::has_get V>
constexpr decltype(auto) get_unchecked(V&& variant_) noexcept {
  return std::forward<V>(variant_).template get<T>();
}

template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  if (variant_.index() != Idx) [[unlikely]] {
    impl::throw_bad_variant_access(variant_.valueless_by_exception());
  }
  return std::forward<V>(variant_).template get<Idx>();
}

template <typename T, impl::has_get V>
constexpr decltype(auto) get(V&& variant_) {
  return std::forward<V>(variant_).template get<T>();
}

template <std::size_t Idx, impl::has_get V>
constexpr auto* get_if(V* variant_) noexcept {
  static_assert(Idx < std::remove_cvref_t<V>::alternatives::size, "index must be in [0, number of alternatives)");
  static_assert(!std::is_void_v<variant_alternative_t<Idx, std::remove_cvref_t<V>>>,
                "alternative type must not be void");

  if (variant_ && variant_->index() == Idx) {
    return std::addressof(slo::get_unchecked<Idx>(*variant_));
  }
  return nullptr;
}

template <typename T, impl::has_get V>
constexpr auto* get_if(V* variant_) noexcept {
  constexpr static std::size_t index = util::index_of<T, typename std::remove_cvref_t<V>::alternatives>;
  static_assert(index < std::remove_cvref_t<V>::alternatives::size, "T must occur exactly once in alternatives");
  return get_if<index>(variant_);
}

template <typename R, typename F, typename... Vs>
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
    using visit_helper = impl::VisitStrategy<strategy>;
    return visit_helper::template visit<R>(std::forward<F>(visitor), std::forward<Vs>(variants)...);
  }
}

template <typename F, typename... Vs>
constexpr decltype(auto) visit(F&& visitor, Vs&&... variants) {
  using return_type = impl::visit_result_t<F, Vs...>;
  return slo::visit<return_type>(std::forward<F>(visitor), std::forward<Vs>(variants)...);
}

namespace impl {
template <template <bool, typename...> class Generator>
struct GeneratorWrapper {
  template <typename... Ts>
  using type = Generator<(... && std::is_trivially_destructible_v<Ts>), Ts...>;
};

template <typename... Ts>
using UnionGenerator = typename util::ConditionalTemplate<sizeof...(Ts) < SLO_TREE_THRESHOLD,
                                                          GeneratorWrapper<RecursiveUnion>::type,
                                                          GeneratorWrapper<TreeUnion>::type>::template type<Ts...>;

template <template <typename...> class Generator, template <template <typename...> class, typename...> class Storage>
struct StorageWrapper {
  template <typename... Ts>
  using type = Storage<Generator, Ts...>;
};

template <typename... Ts>
using VariantStorage =
    typename util::ConditionalTemplate<HAS_IS_WITHIN_LIFETIME && (std::is_standard_layout_v<Ts> && ...),
                                       StorageWrapper<UnionGenerator, InvertedStorage>::type,
                                       StorageWrapper<UnionGenerator, Storage>::type>::template type<Ts...>;
}  // namespace impl

/**
 * @brief This implementation of variant uses a recursively defined union under the hood.
 *
 * @tparam Ts Alternative types
 */
template <typename... Ts>
using NormalVariant = impl::Variant<impl::StorageWrapper<impl::UnionGenerator, impl::Storage>::type, Ts...>;

template <typename... Ts>
using RecursiveVariant =
    impl::Variant<impl::StorageWrapper<impl::GeneratorWrapper<impl::RecursiveUnion>::type, impl::Storage>::type, Ts...>;

template <typename... Ts>
using TreeVariant =
    impl::Variant<impl::StorageWrapper<impl::GeneratorWrapper<impl::TreeUnion>::type, impl::Storage>::type, Ts...>;

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
using InvertedVariant = impl::Variant<impl::StorageWrapper<impl::UnionGenerator, impl::InvertedStorage>::type, Ts...>;

template <typename... Ts>
using InvertedRecursiveVariant =
    impl::Variant<impl::StorageWrapper<impl::GeneratorWrapper<impl::RecursiveUnion>::type, impl::InvertedStorage>::type,
                  Ts...>;

template <typename... Ts>
using InvertedTreeVariant =
    impl::Variant<impl::StorageWrapper<impl::GeneratorWrapper<impl::TreeUnion>::type, impl::InvertedStorage>::type,
                  Ts...>;

/**
 * @brief If the compiler supports `std::is_within_lifetime` or has an intrinsic
 *        that can be used to implement this, this type will use the inverted
 *        storage strategy to reduce padding as much as possible.
 *
 * @tparam Ts Alternative types
 */
template <typename... Ts>
using Variant = impl::Variant<impl::VariantStorage, Ts...>;

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
using Union = impl::Variant<impl::detail::StorageProxy, impl::detail::MemberPtr<Ptrs>...>;

template <typename... Ts, typename... Args>
Variant<Ts...> make_variant(Args&&... args) {
  return {std::forward<Args>(args)...};
}

template <auto... Ptrs, typename... Args>
Union<Ptrs...> make_variant(Args&&... args) {
  return {std::forward<Args>(args)...};
}

struct monostate {};

}  // namespace slo

template <>
struct std::hash<slo::monostate> {
  using result_type   = std::size_t;
  using argument_type = slo::monostate;

  std::size_t operator()(const slo::monostate&) const noexcept {
    constexpr static std::size_t monostate_hash = 0xb03c924ec92d7c6f;
    return monostate_hash;
  }
};

template <template <typename...> class Storage, typename... Ts>
  requires(slo::util::is_hashable<Ts> && ...)
struct std::hash<slo::impl::Variant<Storage, Ts...>> {
  using result_type   = std::size_t;
  using argument_type = slo::impl::Variant<Storage, Ts...>;

  std::size_t operator()(argument_type const& obj) const
      noexcept((std::is_nothrow_invocable_v<std::hash<std::remove_const_t<Ts>>, Ts const&> && ...)) {
    if (obj.valueless_by_exception()) {
      constexpr static std::size_t valueless_hash = 0x22c08c8cbcae8fc4;
      return valueless_hash;
    }
    return obj.index() +
           slo::visit([]<typename T>(T const& value) { return std::hash<std::remove_cvref_t<T>>{}(value); }, obj);
  }
};