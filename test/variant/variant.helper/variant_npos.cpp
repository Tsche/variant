#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/type_name.h>

TEST(Helpers, VariantNpos) {
  EXPECT_EQ(slo::variant_npos, static_cast<std::size_t>(-1ULL));
}
