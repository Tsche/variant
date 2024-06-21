#include <gtest/gtest.h>

#include <slo/variant.h>
#include <common/assertions.h>
#include <common/util.h>
#include <common/fixtures/helpers.h>

TYPED_TEST(Helpers, HoldsAlternative) {
  constexpr get_variant<TypeParam, short, long> variant;
  ASSERT_TRUE(slo::holds_alternative<short>(variant));
  ASSERT_TRUE(!slo::holds_alternative<long>(variant));
}

TYPED_TEST(Helpers, HoldsAlternativeNoexcept){
  const get_variant<TypeParam, int> variant;
  ASSERT_NOEXCEPT(slo::holds_alternative<int>(variant));
}