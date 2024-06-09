#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/type_name.h>
#include <common/util.h>
#include <common/fixtures/helpers.h>

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

TYPED_TEST(Helpers, VariantSize) {
  check_size<get_variant<TypeParam>, 0>();
  check_size<get_variant<TypeParam, int>, 1>();
  check_size<get_variant<TypeParam, int, char, void*>, 3>();
}
