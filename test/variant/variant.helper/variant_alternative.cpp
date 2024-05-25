#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/assertions.h>

template <class Variant, std::size_t Idx, class Expected>
void check_size() {
  // type
  EXPECT_SAME((typename slo::variant_alternative<Idx, Variant>::type), Expected);
  EXPECT_SAME((typename slo::variant_alternative<Idx, const Variant>::type), const Expected);
  EXPECT_SAME((typename slo::variant_alternative<Idx, volatile Variant>::type), volatile Expected);
  EXPECT_SAME((typename slo::variant_alternative<Idx, const volatile Variant>::type), const volatile Expected);

  // alias
  EXPECT_SAME((slo::variant_alternative_t<Idx, Variant>), Expected);
  EXPECT_SAME((slo::variant_alternative_t<Idx, const Variant>), const Expected);
  EXPECT_SAME((slo::variant_alternative_t<Idx, volatile Variant>), volatile Expected);
  EXPECT_SAME((slo::variant_alternative_t<Idx, const volatile Variant>), const volatile Expected);
}

TEST(Helpers, variant_alternative) {
  using V = slo::variant<int, void*, const void*, long double>;
  check_size<V, 0, int>();
  check_size<V, 1, void*>();
  check_size<V, 2, const void*>();
  check_size<V, 3, long double>();
}

// todo ensure out of bounds access causes a static assertion failure