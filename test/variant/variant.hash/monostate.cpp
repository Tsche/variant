#include <gtest/gtest.h>
#include <slo/variant.h>
#include <slo/util/concepts.h>

#include <common/assertions.h>


TEST(Hash, Monostate) {
  auto const hasher = std::hash<slo::monostate>{};

  auto obj1 = slo::monostate{};
  auto const obj2 = slo::monostate{};

  ASSERT_EQ(hasher(obj1), hasher(obj1));
  ASSERT_EQ(hasher(obj1), hasher(obj2));
  ASSERT_EQ(hasher(obj2), hasher(obj1));
  ASSERT_EQ(hasher(obj2), hasher(obj2));
  ASSERT_SAME(decltype(hasher(obj1)), std::size_t);
  ASSERT_NOEXCEPT(hasher(obj1));
  ASSERT_TRUE(slo::util::is_hashable<slo::monostate>);
}