#pragma once

namespace slo {

template <typename F, typename... Vs>
constexpr decltype(auto) visit(F&& visitor, Vs&&... variant);
}

namespace slo::impl {
struct error_type{};
}