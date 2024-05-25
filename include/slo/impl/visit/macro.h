#pragma once
#include <cstddef>
#include <utility>
#include <slo/util/stamp.h>
#include <slo/impl/exception.h>

#include "visit.h"

namespace slo {
template <typename T>
struct variant_size;
}

namespace slo::impl {

template <int I>
struct VisitStrategy;

#define SLO_VISIT_CASE(Idx)                                                                                  \
  case Idx:                                                                                                  \
    if constexpr ((Idx) < VisitImpl<Vs...>::max_index) {                                                             \
      return VisitImpl<Vs...>::template visit<Idx>(std::forward<F>(visitor), std::forward<Vs>(variants)...); \
    }                                                                                                        \
    std::unreachable();

#define SLO_VISIT_STAMP(stamper, n)                                                \
  const auto key = typename VisitImpl<Vs...>::key_type(variants.index()...);                                  \
  switch (static_cast<std::size_t>(key)) {                                                             \
    stamper(0, SLO_VISIT_CASE);                                                    \
    default: throw_bad_variant_access((variants.valueless_by_exception() || ...)); \
  }

#define SLO_GENERATE_STRATEGY(Idx, Count)                                  \
  template <>                                                              \
  struct VisitStrategy<Idx> {                                              \
    template <typename F, typename... Vs>                                  \
    static constexpr decltype(auto) visit(F&& visitor, Vs&&... variants) { \
      SLO_STAMP(Count, SLO_VISIT_STAMP);                                   \
    }                                                                      \
  }

SLO_GENERATE_STRATEGY(1, 4);    // 4^1 potential states
SLO_GENERATE_STRATEGY(2, 16);   // 4^2 potential states
SLO_GENERATE_STRATEGY(3, 64);   // 4^3 potential states
SLO_GENERATE_STRATEGY(4, 256);  // 4^4 potential states

#undef SLO_VISIT_CASE
#undef SLO_VISIT_STAMP
#undef SLO_GENERATE_STRATEGY
}  // namespace slo::impl
