#include <gtest/gtest.h>

#include <slo/variant.h>
#include <common/assertions.h>
#include <type_traits>

struct NonDefaultConstructible{
  NonDefaultConstructible() = delete;
};

struct NotNoexcept {
  NotNoexcept() noexcept(false) {}
};

struct DefaultCtorThrows {
  DefaultCtorThrows() { throw 42; }
};

TEST(Ctor, Default){
  EXPECT_TRUE((std::is_default_constructible_v<slo::Variant<std::monostate>>));
  EXPECT_TRUE((std::is_default_constructible_v<slo::Variant<std::monostate, int>>));
  EXPECT_FALSE((std::is_default_constructible_v<slo::Variant<NonDefaultConstructible, int>>));
}

TEST(Ctor, DefaultNoexcept) {
  EXPECT_TRUE((std::is_nothrow_default_constructible_v<slo::Variant<int>>));
  EXPECT_FALSE((std::is_nothrow_default_constructible_v<slo::Variant<NotNoexcept>>));
}
