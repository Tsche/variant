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

template <int I>
struct VisitStrategy;

template <>
struct VisitStrategy<-1> {
  // TODO dispatch table as fallback for ridiculously large variants
};

template <>
struct VisitStrategy<0> {  // 4^0 potential states
  template <typename>
  static constexpr decltype(auto) visit() {
    return -1;
  }
};

template <>
struct VisitStrategy<1> {  // 4^1 potential states
  template <typename V>
  static constexpr decltype(auto) visit(V&& variant) {
    SLO_STAMP(4, SLO_VISIT_STAMP);
  }
};

template <>
struct VisitStrategy<2> {  // 4^2 potential states
  template <typename V>
  static constexpr decltype(auto) visit(V&& variant) {
    SLO_STAMP(16, SLO_VISIT_STAMP);
  }
};

template <>
struct VisitStrategy<3> {  // 4^3 potential states
  template <typename V>
  static constexpr decltype(auto) visit(V&& variant) {
    SLO_STAMP(64, SLO_VISIT_STAMP);
  }
};

template <>
struct VisitStrategy<4> {  // 4^4 potential states
  template <typename V>
  static constexpr decltype(auto) visit(V&& variant) {
    SLO_STAMP(256, SLO_VISIT_STAMP);
  }
};

template <typename V>
constexpr decltype(auto) visit(V&& variant) {
  using variant_type     = std::remove_cvref_t<V>;
  constexpr int strategy = variant_type::size <= 4     ? 1
                           : variant_type::size <= 16  ? 2
                           : variant_type::size <= 64  ? 3
                           : variant_type::size <= 256 ? 4
                                                       : -1;
  return VisitStrategy<strategy>::visit(std::forward<V>(variant));
}
