#include <gtest/gtest.h>
#include <slo/variant.h>

#include <common/assertions.h>
#include <common/util.h>


template <typename T>
struct Get : testing::Test {};
TYPED_TEST_SUITE(Get, test_variants);

TYPED_TEST(Get, IndexLvalue){
  using variant = get_variant<TypeParam, const int, float>;

  {
    constexpr variant obj(42);
    ASSERT_NOT_NOEXCEPT(slo::get<0>(obj));
    ASSERT_SAME(decltype(slo::get<0>(obj)), const int&);
    static_assert(slo::get<0>(obj) == 42, "Value mismatch");
  }
}
TYPED_TEST(Get, IndexRvalue){}
TYPED_TEST(Get, IndexConstLvalue){}
TYPED_TEST(Get, IndexConstRvalue){}
TYPED_TEST(Get, IndexException){}

TYPED_TEST(Get, TypeLvalue){}
TYPED_TEST(Get, TypeRvalue){}
TYPED_TEST(Get, TypeConstLvalue){}
TYPED_TEST(Get, TypeConstRvalue){}