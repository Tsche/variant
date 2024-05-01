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

using test_types = testing::Types<test_type<slo::InvertedVariant, 5>,   // small union   -> recursive
                                  test_type<slo::InvertedVariant, 50>,  // big union     -> tree
                                  test_type<slo::RegularVariant, 5>,    // small variant -> recursive
                                  test_type<slo::RegularVariant, 50>    // big variant   -> tree
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
  auto obj = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_FALSE(qualifiers.is_const);
        ASSERT_FALSE(qualifiers.is_rvalue);
      },
      obj);

}
TYPED_TEST(VisitTest, const_lvref) {
  const auto obj = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_TRUE(qualifiers.is_const);
        ASSERT_FALSE(qualifiers.is_rvalue);
      },
      obj);
}

TYPED_TEST(VisitTest, rvref) {
  auto obj = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_FALSE(qualifiers.is_const);
        ASSERT_TRUE(qualifiers.is_rvalue);
      },
      std::move(obj));
}

TYPED_TEST(VisitTest, const_rvref) {
  const auto obj = TypeParam{std::in_place_index<1>};
  slo::visit(
      [](auto&& obj) {
        auto qualifiers = Qualifiers(std::forward<decltype(obj)>(obj));
        ASSERT_TRUE(qualifiers.is_const);
        ASSERT_TRUE(qualifiers.is_rvalue);
      },
      std::move(obj));
}
