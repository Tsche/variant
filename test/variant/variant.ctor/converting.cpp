#include <type_traits>
#include <string>

#include <gtest/gtest.h>
#include <common/util.h>
#include <slo/variant.h>

TEST(ConvertingConstructor, ExactMatch) {
  using test_variant = slo::Variant<Constant<0>, Constant<1>, Constant<2>>;
  ASSERT_TRUE((std::is_constructible_v<test_variant, Constant<1>>));

  ASSERT_EQ((slo::impl::selected_index<Constant<1>, slo::util::TypeList<Constant<0>, Constant<1>, Constant<2>>>), 1);
}

TEST(ConvertingConstructor, Convertible) {
  ASSERT_TRUE((std::is_constructible_v<slo::Variant<bool>, std::true_type>));

  // sanity check
  ASSERT_FALSE((std::is_constructible_v<slo::Variant<bool>, std::string>));
  ASSERT_FALSE((std::is_constructible_v<slo::Variant<bool>, std::nullptr_t>));
}

TEST(ConvertingConstructor, TypePromotion) {
  ASSERT_TRUE((std::is_constructible_v<slo::Variant<long>, short>));

  // check if it selects the right alternative
  ASSERT_EQ((slo::impl::selected_index<short, slo::util::TypeList<char, long>>), 1);
}

TEST(ConvertingConstructor, Invalid) {
  // ambiguous
  ASSERT_EQ((slo::impl::selected_index<int, slo::util::TypeList<int, int>>), slo::variant_npos);
  ASSERT_EQ((slo::impl::selected_index<int, slo::util::TypeList<long, long long>>), slo::variant_npos);
  ASSERT_FALSE((std::is_constructible_v<slo::Variant<int, int>, int>));
  ASSERT_FALSE((std::is_constructible_v<slo::Variant<long, long long>, int>));


  // not contained
  ASSERT_EQ((slo::impl::selected_index<float, slo::util::TypeList<char, int>>), slo::variant_npos);
  ASSERT_FALSE((std::is_constructible_v<slo::Variant<char, int>, float>));

  // narrowing
  ASSERT_EQ((slo::impl::selected_index<long, slo::util::TypeList<char, short>>), slo::variant_npos);
  ASSERT_FALSE((std::is_constructible_v<slo::Variant<char, short>, long>));
}
