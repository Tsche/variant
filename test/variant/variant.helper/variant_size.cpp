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

TEST(Helpers, variant_size) {
  check_size<slo::variant<>, 0>();
  check_size<slo::variant<int>, 1>();
  check_size<slo::variant<int, char, void*>, 3>();
}
