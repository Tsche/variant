#include <gtest/gtest.h>
#include <cstddef>

#include <slo/variant.h>
#include <common/assertions.h>

template <class Variant, std::size_t Idx, class Expected>
void check_alternatives() {
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

union AlternativeUnion {
  int member_0;
  void* member_1;
  const void* member_2;
  long double member_3;
};

TEST_T(Helpers,
       VariantAlternative,
       slo::RecursiveVariant<int, void*, const void*, long double>,
       slo::TreeVariant<int, void*, const void*, long double>,
       slo::InvertedRecursiveVariant<int, void*, const void*, long double>,
       slo::InvertedTreeVariant<int, void*, const void*, long double>,
       slo::Union<&AlternativeUnion::member_0,
                  &AlternativeUnion::member_1,
                  &AlternativeUnion::member_2,
                  &AlternativeUnion::member_3>) {
  check_alternatives<TypeParam, 0, int>();
  check_alternatives<TypeParam, 1, void*>();
  check_alternatives<TypeParam, 2, const void*>();
  check_alternatives<TypeParam, 3, long double>();
}

// todo ensure out of bounds access causes a static assertion failure