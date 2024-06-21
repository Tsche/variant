#pragma once
#include <concepts>
#include <cstddef>
#include <array>
#include <utility>
#include <type_traits>

#include <slo/impl/concepts.h>

namespace slo {
template <std::size_t, typename>
struct variant_alternative;

template <typename>
struct variant_size;

template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get_unchecked(V&&) noexcept;
}  // namespace slo

namespace slo::impl {
template <int I>
struct VisitStrategy;

template <std::size_t... Dimensions>
struct Key {
  constexpr static std::size_t size      = sizeof...(Dimensions);
  constexpr static std::size_t max_index = (Dimensions * ... * 1);

  static consteval auto generate_offsets() {
    std::array<std::size_t, size> result = {};
    constexpr std::size_t dimensions[]   = {Dimensions...};
    result[0]                            = 1;
    for (std::size_t idx = 1; idx < size; ++idx) {
      result[idx] = dimensions[idx - 1] * result[idx - 1];
    }

    return result;
  }

  constexpr static auto offsets = generate_offsets();

  std::size_t index                             = static_cast<std::size_t>(-1);
  std::size_t subindices[sizeof...(Dimensions)] = {};

  constexpr explicit Key(std::size_t index_) noexcept : index(index_) {
    std::size_t key                    = index_;
    constexpr std::size_t dimensions[] = {Dimensions...};
    for (std::size_t idx = 0; idx < size; ++idx) {
      subindices[idx] = key % dimensions[idx];
      key /= dimensions[idx];
    }
  }

  constexpr explicit Key(std::convertible_to<std::size_t> auto... subindices_) noexcept
    requires(sizeof...(subindices_) > 1)
      : index(0)
      , subindices(subindices_...) {
    static_assert(sizeof...(subindices_) == size, "Number of indices must match the number of dimensions.");
    for (std::size_t idx = 0; idx < size; ++idx) {
      index += subindices[idx] * offsets[idx];
    }
  }

  constexpr explicit operator std::size_t() const noexcept { return index; }
  constexpr std::size_t operator[](std::size_t position) const noexcept { return subindices[position]; }
  friend constexpr auto operator<=>(Key const& self, std::size_t other) { return self.index <=> other; }
  friend constexpr bool operator==(Key const& self, std::size_t other) { return self.index == other; }
};

template <typename... Variants>  // possibly cv qualified
struct VisitImpl {
  using key_type                         = Key<variant_size<std::remove_cvref_t<Variants>>::value...>;
  static constexpr std::size_t max_index = key_type::max_index;

  template <key_type Tag, typename = std::make_index_sequence<sizeof...(Variants)>>
  struct Dispatch;

  template <key_type Tag, std::size_t... Idx>
  struct Dispatch<Tag, std::index_sequence<Idx...>> {
    template <typename F, typename... U>
    [[gnu::always_inline]]
    constexpr static decltype(auto) visit(F&& visitor, U&&... variants) {
      return std::forward<F>(visitor)(slo::get_unchecked<Tag[Idx]>(std::forward<U>(variants))...);
    }
  };

  template <std::size_t Idx, typename F, typename... U>
  [[gnu::always_inline]]
  constexpr static decltype(auto) visit(F&& visitor, U&&... variants) {
    return Dispatch<key_type{Idx}>::visit(std::forward<F>(visitor), std::forward<U>(variants)...);
  }
};

template <typename Variant>
struct VisitImpl<Variant> {
  using key_type                         = std::size_t;
  static constexpr std::size_t max_index = variant_size<std::remove_cvref_t<Variant>>::value;

  template <std::size_t Idx, typename F, typename U>
  [[gnu::always_inline]] constexpr static decltype(auto) visit(F&& visitor, U&& variant) {
    return std::forward<F>(visitor)(slo::get_unchecked<Idx>(std::forward<U>(variant)));
  }
};

// non-exhaustive result type check
template <typename F, typename... Vs>
// TODO: Vs... might be derived from variant
using visit_result_t = std::invoke_result_t<F, typename slo::variant_alternative<0, std::remove_cvref_t<Vs>>::type...>;
}  // namespace slo::impl