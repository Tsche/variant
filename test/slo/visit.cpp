#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <concepts>
#include <type_traits>
#include <slo/variant.h>

template <typename T>
struct VisitTest : public testing::Test {
  using type = T;
};

template <auto>
struct Constant {};

template <template <typename...> class T, std::size_t... Idx>
auto get_test_type(std::index_sequence<Idx...>) {
  return std::type_identity<T<Constant<Idx>...>>{};
}
template <template <typename...> class T, std::size_t Count>
using test_type = typename decltype(get_test_type<T>(std::make_index_sequence<Count>()))::type;

union Bare {
  Constant<0> alt_0;
  Constant<1> alt_1;
  Constant<2> alt_2;
  Constant<3> alt_3;
  Constant<4> alt_4;
};

using test_types = testing::Types<test_type<slo::InvertedVariant, 5>,   // small union   -> recursive
                                  test_type<slo::InvertedVariant, 50>,  // big union     -> tree
                                  test_type<slo::NormalVariant, 5>,    // small variant -> recursive
                                  test_type<slo::NormalVariant, 50>,    // big variant   -> tree
                                  slo::Union<&Bare::alt_0, &Bare::alt_1, &Bare::alt_2, &Bare::alt_3, &Bare::alt_4>
                                  >;
TYPED_TEST_SUITE(VisitTest, test_types);

struct Qualifiers {
  bool is_const;
  bool is_rvalue;

  Qualifiers(Qualifiers const&)            = default;
  Qualifiers(Qualifiers&&)                 = default;
  Qualifiers()                             = default;
  ~Qualifiers()                            = default;
  Qualifiers& operator=(Qualifiers const&) = default;
  Qualifiers& operator=(Qualifiers&&)      = default;

  template <typename T>
    requires(!std::same_as<std::remove_cvref_t<T>, Qualifiers>)
  explicit Qualifiers(T&&)
      : is_const(std::is_const_v<std::remove_reference_t<T>>)
      , is_rvalue(std::is_rvalue_reference_v<T&&>) {}
};

TYPED_TEST(VisitTest, lvref) {
  auto variant = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_FALSE(qualifiers.is_const);
        ASSERT_FALSE(qualifiers.is_rvalue);
      },
      variant);

}
TYPED_TEST(VisitTest, const_lvref) {
  const auto variant = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_TRUE(qualifiers.is_const);
        ASSERT_FALSE(qualifiers.is_rvalue);
      },
      variant);
}

TYPED_TEST(VisitTest, rvref) {
  auto variant = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_FALSE(qualifiers.is_const);
        ASSERT_TRUE(qualifiers.is_rvalue);
      },
      std::move(variant));
}

TYPED_TEST(VisitTest, const_rvref) {
  const auto variant = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_TRUE(qualifiers.is_const);
        ASSERT_TRUE(qualifiers.is_rvalue);
      },
      std::move(variant));
}
