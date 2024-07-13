#include <type_traits>
#include <string>

#include <gtest/gtest.h>
#include <common/util.h>
#include <slo/variant.h>

template <typename T>
struct ConvertingConstructor : testing::Test {};
TYPED_TEST_SUITE(ConvertingConstructor, test_variants);

TYPED_TEST(ConvertingConstructor, ExactMatch) {
  using test_variant = get_variant<TypeParam, Constant<0>, Constant<1>, Constant<2>>;
  ASSERT_TRUE((std::is_constructible_v<test_variant, Constant<1>>));

  ASSERT_EQ((slo::impl::selected_index<Constant<1>, slo::util::TypeList<Constant<0>, Constant<1>, Constant<2>>>), 1);
}

TYPED_TEST(ConvertingConstructor, Convertible) {
  using test_variant = get_variant<TypeParam, bool>;
  ASSERT_TRUE((std::is_constructible_v<test_variant, std::true_type>));

  // sanity check
  ASSERT_FALSE((std::is_constructible_v<test_variant, std::string>));
  ASSERT_FALSE((std::is_constructible_v<test_variant, std::nullptr_t>));
}

TYPED_TEST(ConvertingConstructor, TypePromotion) {
  using test_variant = get_variant<TypeParam, long>;
  ASSERT_TRUE((std::is_constructible_v<test_variant, short>));

  // check if it selects the right alternative
  ASSERT_EQ((slo::impl::selected_index<short, slo::util::TypeList<char, long>>), 1);
}

TYPED_TEST(ConvertingConstructor, Invalid) {
  using ambiguous_variant = get_variant<TypeParam, int, int>;
  using test_variant = get_variant<TypeParam, short, int>;

  // ambiguous
  ASSERT_EQ((slo::impl::selected_index<int, slo::util::TypeList<int, int>>), slo::variant_npos);
  ASSERT_EQ((slo::impl::selected_index<int, slo::util::TypeList<long, long long>>), slo::variant_npos);  
  ASSERT_FALSE((std::is_constructible_v<ambiguous_variant, int>));

  // not contained
  ASSERT_EQ((slo::impl::selected_index<float, slo::util::TypeList<short, long>>), slo::variant_npos);
  ASSERT_FALSE((std::is_constructible_v<test_variant, float>));

  // narrowing
  ASSERT_EQ((slo::impl::selected_index<long long, slo::util::TypeList<short, int>>), slo::variant_npos);
  ASSERT_FALSE((std::is_constructible_v<test_variant, long long>));
}
