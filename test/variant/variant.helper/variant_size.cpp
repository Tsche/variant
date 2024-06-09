#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/type_name.h>

template <class V, std::size_t E>
void check_size() {
  EXPECT_EQ(slo::variant_size<V>::value, E);
  EXPECT_EQ(slo::variant_size<V const>::value, E);
  EXPECT_EQ(slo::variant_size<V volatile>::value, E);
  EXPECT_EQ(slo::variant_size<V const volatile>::value, E);

  EXPECT_EQ(slo::variant_size_v<V>, E);
  EXPECT_EQ(slo::variant_size_v<V const>, E);
  EXPECT_EQ(slo::variant_size_v<V volatile>, E);
  EXPECT_EQ(slo::variant_size_v<V const volatile>, E);
}

TEST(Helpers, VariantSize) {
  check_size<slo::NormalVariant<>, 0>();
  check_size<slo::InvertedVariant<>, 0>();

  check_size<slo::NormalVariant<int>, 1>();
  check_size<slo::NormalVariant<int, char, void*>, 3>();
  check_size<slo::InvertedVariant<int>, 1>();
  check_size<slo::InvertedVariant<int, char, void*>, 3>();
}
