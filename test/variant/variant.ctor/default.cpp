#include <type_traits>
#include <gtest/gtest.h>

#include <slo/variant.h>
#include <common/util.h>

template <typename T>
struct DefaultCtor : testing::Test {};
TYPED_TEST_SUITE(DefaultCtor, test_variants);

struct NonDefaultConstructible {
  NonDefaultConstructible() = delete;
};

struct NotNoexcept {
  NotNoexcept() noexcept(false) {}
};

struct DefaultCtorThrows {
  DefaultCtorThrows() { throw 42; }
};

TYPED_TEST(DefaultCtor, Constructible) {
  EXPECT_TRUE((std::is_default_constructible_v<get_variant<TypeParam, std::monostate>>));
  EXPECT_TRUE((std::is_default_constructible_v<get_variant<TypeParam, std::monostate, int>>));
  EXPECT_FALSE((std::is_default_constructible_v<get_variant<TypeParam, NonDefaultConstructible, int>>));
}

TYPED_TEST(DefaultCtor, Noexcept) {
  EXPECT_TRUE((std::is_nothrow_default_constructible_v<get_variant<TypeParam, int>>));
  EXPECT_FALSE((std::is_nothrow_default_constructible_v<get_variant<TypeParam, NotNoexcept>>));
}
