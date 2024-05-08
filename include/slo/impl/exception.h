#pragma once

#include <exception>
namespace slo {

namespace impl{
  [[noreturn]] void throw_bad_variant_access(bool);
}

class bad_variant_access : public std::exception {
  friend void impl::throw_bad_variant_access(bool);
  char const* reason = "bad variant access";
  explicit bad_variant_access(const char* message) noexcept : reason(message) {}
public:
  bad_variant_access() noexcept = default;
  [[nodiscard]] const char* what() const noexcept override {
    return reason;
  }
};
namespace impl {
  [[noreturn]] inline void throw_bad_variant_access(bool valueless){
    if (valueless) {
      throw slo::bad_variant_access("variant is valueless");
    }
    throw slo::bad_variant_access("wrong index for variant");
  }
}
}