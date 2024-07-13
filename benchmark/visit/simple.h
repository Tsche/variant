#pragma once
#include <utility>
#include <type_traits>
#include <slo/util/stamp.h>

template <std::size_t Idx>
int h() {
  return Idx;
}

#define SLO_VISIT_CASE(Idx)                               \
  case Idx:                                               \
    if constexpr ((Idx) < std::remove_cvref_t<V>::size) { \
      return h<Idx>();                                    \
    }                                                     \
    std::unreachable();

#define SLO_VISIT_STAMP(stamper, n) \
  switch (variant.index()) {        \
    stamper(0, SLO_VISIT_CASE);     \
    default: std::unreachable();    \
  }

template <typename V>
constexpr decltype(auto) visit(V&& variant) {
  SLO_STAMP(256, SLO_VISIT_STAMP);
}