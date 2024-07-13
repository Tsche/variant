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
  ASSERT_TRUE((std::is_default_constructible_v<get_variant<TypeParam, std::monostate>>));
  ASSERT_TRUE((std::is_default_constructible_v<get_variant<TypeParam, std::monostate, int>>));
  ASSERT_FALSE((std::is_default_constructible_v<get_variant<TypeParam, NonDefaultConstructible, int>>));
}

TYPED_TEST(DefaultCtor, Noexcept) {
  ASSERT_TRUE((std::is_nothrow_default_constructible_v<get_variant<TypeParam, int>>));
  ASSERT_FALSE((std::is_nothrow_default_constructible_v<get_variant<TypeParam, NotNoexcept>>));
}
