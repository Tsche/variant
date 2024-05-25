#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/assertions.h>

TEST(Helpers, holds_alternative) {
  constexpr slo::Variant<short, long> variant;
  EXPECT_TRUE(slo::holds_alternative<short>(variant));
  EXPECT_TRUE(!slo::holds_alternative<long>(variant));
}

TEST(Helpers, holds_alternative_noexcept){
  const slo::Variant<int> variant;
  EXPECT_NOEXCEPT(slo::holds_alternative<int>(variant));
}