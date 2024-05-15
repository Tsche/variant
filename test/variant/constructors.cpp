#include <concepts>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <slo/variant.h>

template <auto V>
struct Constant {
  static constexpr auto value = V;
};

TEST(ConvertingConstructor, exact_match) {
  ASSERT_EQ((slo::impl::selected_index<Constant<1>, slo::util::TypeList<Constant<0>, Constant<1>, Constant<2>>>), 1);
  auto variant = slo::variant<Constant<0>, Constant<1>, Constant<2>>(Constant<1>{});
  auto result  = slo::visit([](auto obj) { return obj.value; }, variant);
  ASSERT_EQ(result, 1);
}

TEST(ConvertingConstructor, type_promotion) {
  ASSERT_EQ((slo::impl::selected_index<short, slo::util::TypeList<char, long>>), 1);

  auto variant = slo::variant<char, long>(short{2});
  slo::visit([]<typename T>(T) { ASSERT_TRUE((std::same_as<T, long>)); }, variant);
}

TEST(ConvertingConstructor, invalid) {
  // ambiguous
  ASSERT_EQ((slo::impl::selected_index<int, slo::util::TypeList<int, int>>), slo::variant_npos);

  // not contained
  ASSERT_EQ((slo::impl::selected_index<float, slo::util::TypeList<char, int>>), slo::variant_npos);

  // narrowing
  ASSERT_EQ((slo::impl::selected_index<long, slo::util::TypeList<char, short>>), slo::variant_npos);
}
