#pragma once
#include <cstddef>
#include <concepts>
#include <memory>
#include <utility>
#include <type_traits>

#include <slo/util/stamp.h>
#include "concepts.h"

namespace slo {
template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&&);

namespace impl {
#if !defined(_MSC_VER)
template <typename Variant, typename F, std::size_t... Is>
constexpr decltype(auto) visit(Variant&& variant, F visitor, std::index_sequence<Is...>) {
  using return_type = std::common_type_t<decltype(visitor(slo::get<Is>(std::forward<Variant>(variant))))...>;
  // using return_type = std::common_type_t<std::invoke_result_t<F, std::invoke_result_t<decltype(slo::get<Is, U>),
  // U>>...>;
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
#else
#  define SLO_VISIT_CASE(Idx)                                          \
    case Idx:                                                          \
      if constexpr (Idx <= variant.size) {                             \
        return visitor(slo::get<Idx>(std::forward<Variant>(variant))); \
      }                                                                \
      std::unreachable();

#  define SLO_VISIT_STAMP(stamper, n) stamper(0, _STL_CASE);

template <typename Variant, typename F, std::size_t... Is>
constexpr decltype(auto) visit(Variant&& variant, F visitor) {
  SLO_STAMP(64, SLO_VISIT_STAMP);
}
#endif
}  // namespace impl
}  // namespace slo
