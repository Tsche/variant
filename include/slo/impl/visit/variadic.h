#pragma once
#include <cstddef>
#include <concepts>
#include <memory>
#include <utility>
#include <type_traits>

#include <slo/impl/concepts.h>
#include <slo/impl/exception.h>

namespace slo {
template <std::size_t Idx, impl::has_get V>
constexpr decltype(auto) get(V&&);

namespace impl::variadic {

template <typename F, typename V, std::size_t... Is>
constexpr decltype(auto) visit(F&& visitor, V&& variant, std::index_sequence<Is...>) {
  using return_type =
      std::common_type_t<std::invoke_result_t<F&&, std::invoke_result_t<decltype(slo::get<Is, V>), V&&>>...>;
  bool success = false;

  if constexpr (std::same_as<return_type, void>) {
    success =
        ((variant.index() == Is ? std::forward<F>(visitor)(slo::get<Is>(std::forward<V>(variant))), true : false) ||
         ...);
    if (success) {
      return;
    }
  } else {
    union {
      char dummy;
      return_type value;
    } ret{};

    success = ((variant.index() == Is
                ? std::construct_at(&ret.value, std::forward<F>(visitor)(slo::get<Is>(std::forward<V>(variant)))),
                true : false) ||
               ...);

    if (success) {
      return ret.value;
    }
  }
  throw_bad_variant_access(variant.index() == variant.npos);
}

}  // namespace impl::variadic
}  // namespace slo
