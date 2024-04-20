#pragma once
#include <utility>
#include <memory>
#include <cstddef>
#include <slo/util/list.h>
#include "common.h"

namespace slo {
    namespace detail{
template <typename Variant, typename F, std::size_t... Is>
constexpr decltype(auto) visit_impl(Variant&& variant, F visitor,
                          std::index_sequence<Is...>) {
    // assert(variant.index() != variant.npos);
    using return_type = std::common_type_t<decltype(visitor(slo::get<Is>(std::forward<Variant>(variant))))...>;
    if constexpr (std::same_as<return_type, void>) {
        (void)((variant.index() == Is ? visitor(slo::get<Is>(std::forward<Variant>(variant))), true : false) ||
               ...);
    } else {
        union {
            char dummy;
            return_type value;
        } ret{};

        bool const success =
            ((variant.index() == Is
              ? std::construct_at(&ret.value, visitor(slo::get<Is>(std::forward<Variant>(variant)))),
              true : false) ||
             ...);

        if (success) {
            return ret.value;
        }
        std::unreachable();
    }
}
}


template <typename F, typename Variant>
constexpr decltype(auto) visit(F visitor, Variant&& variant) {
    return detail::visit_impl(std::forward<Variant>(variant), visitor, util::to_index_sequence<Variant>{});
}
}