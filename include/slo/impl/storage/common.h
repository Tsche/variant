#pragma once

namespace slo {

template <typename F, typename Variant>
constexpr decltype(auto) visit(F&& visitor, Variant&& variant);
}

namespace slo::impl {
struct error_type{};
}