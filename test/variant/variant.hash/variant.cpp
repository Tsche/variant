#include <utility>
#include <gtest/gtest.h>
#include <slo/variant.h>

#include <common/assertions.h>
#include <common/util.h>
template <typename T>
struct Hash : testing::Test {};
TYPED_TEST_SUITE(Hash, test_variants);

TYPED_TEST(Hash, DuplicateElements) {
  // tests whether the index is used for the hash
  using variant_type  = get_variant<TypeParam, slo::monostate, slo::monostate>;
  auto const hasher   = std::hash<variant_type>{};
  auto const variant1 = variant_type(std::in_place_index<0>);
  auto const variant2 = variant_type(std::in_place_index<1>);

  ASSERT_EQ(hasher(variant1), hasher(variant1));
  ASSERT_EQ(hasher(variant2), hasher(variant2));
  ASSERT_NE(hasher(variant1), hasher(variant2));
}

TYPED_TEST(Hash, Copy) {
  using variant_type  = get_variant<TypeParam, int, long>;
  auto const hasher   = std::hash<variant_type>{};
  auto const obj      = variant_type(std::in_place_index<0>, 42);
  auto const obj_copy = obj;
  auto const obj2     = variant_type(std::in_place_index<0>, 100);

  ASSERT_EQ(hasher(obj), hasher(obj));
  ASSERT_EQ(hasher(obj_copy), hasher(obj_copy));
  ASSERT_EQ(hasher(obj2), hasher(obj2));

  ASSERT_EQ(hasher(obj), hasher(obj_copy));
  ASSERT_NE(hasher(obj), hasher(obj2));
  ASSERT_NE(hasher(obj_copy), hasher(obj2));

  ASSERT_SAME(decltype(hasher(obj)), std::size_t);
}

TYPED_TEST(Hash, Valueless) {
  // TODO
}