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
  // template <typename F, typename... Vs, std::size_t... Idx>
  // constexpr static auto make_fptr_array(std::index_sequence<Idx...>) {
  //   using function_type = visit_result_t<F, Vs...> (*)(F, Vs...);
  //   return std::array<function_type, sizeof...(Idx)>{&VisitImpl<Vs...>::template visit<Idx, F, Vs...>...};
  // }
  // template <typename F, typename... Vs>
  // static constexpr auto fptr_array = make_fptr_array<F, Vs...>(std::index_sequence_for<Vs...>{});

  template <typename F, typename... Vs, std::size_t... Idx>
  constexpr static decltype(auto) visit_impl(std::index_sequence<Idx...>, F&& visitor, Vs&&... variants) {
    using Impl       = VisitImpl<Vs...>;
    auto const index = typename Impl::key_type(variants.index()...);

    using function_type                       = visit_result_t<F, Vs...> (*)(F&&, Vs&&...);
    static constexpr function_type dispatch[] = {&Impl::template visit<Idx>...};
    if (index < Impl::max_index) {
      return dispatch[std::size_t{index}](std::forward<F>(visitor), std::forward<Vs>(variants)...);
    }
  }

  template <typename F, typename... Vs>
  constexpr static decltype(auto) visit(F&& visitor, Vs&&... variants) {
    constexpr auto states = VisitImpl<Vs...>::max_index;
    return visit_impl(std::make_index_sequence<states>{}, std::forward<F>(visitor), std::forward<Vs>(variants)...);
  }
};

}  // namespace slo::impl