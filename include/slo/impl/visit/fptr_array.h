#pragma once
#include <cstddef>
#include <utility>
#include <array>
#include "visit.h"
namespace slo::impl {

template <int>
struct VisitStrategy;

template <>
struct VisitStrategy<-1> {
  template <typename R, typename F, typename... Vs, std::size_t... Idx>
  constexpr static decltype(auto) visit_impl(std::index_sequence<Idx...>, F&& visitor, Vs&&... variants) {
    using Impl       = VisitImpl<Vs...>;
    auto const index = typename Impl::key_type(variants.index()...);

    using function_type                       = R (*)(F&&, Vs&&...);
    static constexpr function_type dispatch[] = {&Impl::template visit<Idx>...};
    if (index < Impl::max_index) { // TODO should already be checked at this point
      return dispatch[std::size_t{index}](std::forward<F>(visitor), std::forward<Vs>(variants)...);
    }
  }

  template <typename R, typename F, typename... Vs>
  constexpr static decltype(auto) visit(F&& visitor, Vs&&... variants) {
    constexpr auto states = VisitImpl<Vs...>::max_index;
    return visit_impl<R>(std::make_index_sequence<states>{}, std::forward<F>(visitor), std::forward<Vs>(variants)...);
  }
};

}  // namespace slo::impl