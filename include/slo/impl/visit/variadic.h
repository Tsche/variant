#pragma once
#include <cstddef>
#include <concepts>
#include <memory>
#include <utility>

#include <slo/impl/concepts.h>
#include <slo/impl/exception.h>

#include "visit.h"
namespace slo::impl {
template <>
struct VisitStrategy<0> {
  // fold
  template <typename F, typename... Vs, std::size_t... Idx>
  constexpr static decltype(auto) visit_impl(std::index_sequence<Idx...>, F&& visitor, Vs&&... variants) {
    using return_type = visit_result_t<F, Vs...>;
    auto index        = typename VisitImpl<Vs...>::key_type{variants.index()...};
    bool success      = false;
    if constexpr (std::same_as<return_type, void>) {
      success = ((index == Idx
                  ? VisitImpl<Vs...>::template visit<Idx>(std::forward<F>(visitor), std::forward<Vs>(variants)...),
                  true : false) ||
                 ...);
      if (success) {
        return;
      }
    } else {
      union {
        char dummy;
        return_type value;
      } ret{};

      success =
          ((index == Idx ? std::construct_at(&ret.value, VisitImpl<Vs...>::template visit<Idx>(
                                                             std::forward<F>(visitor), std::forward<Vs>(variants)...)),
            true         : false) ||
           ...);

      if (success) {
        return ret.value;
      }
    }

    throw_bad_variant_access((variants.valueless_by_exception() || ...));
  }

  template <typename F, typename... Vs>
  constexpr static decltype(auto) visit(F&& visitor, Vs&&... variants) {
    constexpr auto states = VisitImpl<Vs...>::max_index;
    return visit_impl(std::make_index_sequence<states>{}, std::forward<F>(visitor), std::forward<Vs>(variants)...);
  }
};

}  // namespace slo::impl
