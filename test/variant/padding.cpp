#include <type_traits>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define SLO_HIDE_IN_PADDING ON
#include <slo/variant.h>


TEST(Padding, Inverted) {
  using normal = std::variant<char[7], float>;
  using compressed = slo::InvertedVariant<char[7], float>;
  ASSERT_LT(sizeof(compressed), sizeof(normal));
}

union StandardLayout {
  char first[5];
  float second;
  private:
  char dummy;
};

TEST(Padding, Proxy) {
  using normal = std::variant<char[5], float>;
  using compressed = slo::Union<&StandardLayout::first, &StandardLayout::second>;
  
  ASSERT_LT(sizeof(compressed), sizeof(normal));
}

struct NotStandardLayout {
  [[no_unique_address]]  char data[5];
  NotStandardLayout() = default;
private:
  char dummy;
};
static_assert(!std::is_standard_layout_v<NotStandardLayout>);

union Test2 {
  NotStandardLayout first;
  float second;
};

TEST(Padding, non_standard_layout) {
  using normal = std::variant<NotStandardLayout, float>;
  using compressed = slo::Union<&Test2::first, &Test2::second>;
  ASSERT_LT(sizeof(compressed), sizeof(normal));
}