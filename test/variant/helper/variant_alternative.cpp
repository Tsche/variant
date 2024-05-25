#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/type_name.h>

#define DEPAREN(X) ESC(ISH X)
#define ISH(...)   ISH __VA_ARGS__
#define ESC(...)   ESC_(__VA_ARGS__)
#define ESC_(...)  VAN##__VA_ARGS__
#define VANISH

#define EXPECT_SAME(first, second)                                                                                 \
  EXPECT_TRUE((std::is_same_v<DEPAREN(first), DEPAREN(second)>)) << "  Actual: " << nameof<DEPAREN(first)> << '\n' \
                                                                 << "Expected: " << nameof<DEPAREN(second)>

template <class V, std::size_t I, class E>
void check_size() {
  // type
  EXPECT_SAME((typename slo::variant_alternative<I, V>::type), E);
  EXPECT_SAME((typename slo::variant_alternative<I, const V>::type), const E);
  EXPECT_SAME((typename slo::variant_alternative<I, volatile V>::type), volatile E);
  EXPECT_SAME((typename slo::variant_alternative<I, const volatile V>::type), const volatile E);

  // alias
  EXPECT_SAME((slo::variant_alternative_t<I, V>), E);
  EXPECT_SAME((slo::variant_alternative_t<I, const V>), const E);
  EXPECT_SAME((slo::variant_alternative_t<I, volatile V>), volatile E);
  EXPECT_SAME((slo::variant_alternative_t<I, const volatile V>), const volatile E);
}

TEST(Helpers, variant_alternative) {
  using V = slo::variant<int, void*, const void*, long double>;
  check_size<V, 0, int>();
  check_size<V, 1, void*>();
  check_size<V, 2, const void*>();
  check_size<V, 3, long double>();
}

//todo ensure out of bounds access causes a static assertion failure