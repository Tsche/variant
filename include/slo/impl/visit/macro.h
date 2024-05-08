#pragma once
#include <cstddef>
#include <utility>
#include <type_traits>
#include <slo/util/stamp.h>
#include <slo/impl/concepts.h>
#include <slo/impl/exception.h>

namespace slo {
template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&&);

template <typename T>
struct variant_size;

namespace impl::macro {

#define SLO_VISIT_CASE(Idx)                                                     \
  case Idx:                                                                     \
    if constexpr ((Idx) < variant_size<std::remove_cvref_t<V>>::value) {        \
      return std::forward<F>(visitor)(slo::get<Idx>(std::forward<V>(variant))); \
    }                                                                           \
    std::unreachable();

#define SLO_VISIT_STAMP(stamper, n)                                     \
  switch (variant.index()) {                                            \
    stamper(0, SLO_VISIT_CASE);                                         \
    default: throw_bad_variant_access(variant.index() == variant.npos); \
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
  template <typename F, typename V>
  static constexpr decltype(auto) visit(F&& visitor, V&& variant) {
    SLO_STAMP(4, SLO_VISIT_STAMP);
  }
};

template <>
struct VisitStrategy<2> {  // 4^2 potential states
  template <typename F, typename V>
  static constexpr decltype(auto) visit(F&& visitor, V&& variant) {
    SLO_STAMP(16, SLO_VISIT_STAMP);
  }
};

template <>
struct VisitStrategy<3> {  // 4^3 potential states
  template <typename F, typename V>
  static constexpr decltype(auto) visit(F&& visitor, V&& variant) {
    SLO_STAMP(64, SLO_VISIT_STAMP);
  }
};

template <>
struct VisitStrategy<4> {  // 4^4 potential states
  template <typename F, typename V>
  static constexpr decltype(auto) visit(F&& visitor, V&& variant) {
    SLO_STAMP(256, SLO_VISIT_STAMP);
  }
};

#undef SLO_VISIT_CASE
#undef SLO_VISIT_STAMP

template <typename F, typename V>
constexpr decltype(auto) visit(F&& visitor, V&& variant) {
  constexpr auto size = variant_size<std::remove_cvref_t<V>>::value;
  // clang-format off
  constexpr int strategy = size <= 4     ? 1
                           : size <= 16  ? 2
                           : size <= 64  ? 3
                           : size <= 256 ? 4
                                         : -1;
  // clang-format on

  return VisitStrategy<strategy>::visit(std::forward<F>(visitor), std::forward<V>(variant));
}
}  // namespace impl::macro
}  // namespace slo
