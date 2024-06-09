#include <type_traits>
#include <exception>
#include <variant>

#include <gtest/gtest.h>
#include <slo/variant.h>

TEST(Exception, BadVariantAccess) {
  EXPECT_TRUE((std::is_base_of_v<std::exception, slo::bad_variant_access>));
  EXPECT_TRUE(noexcept(std::bad_variant_access{})) << "bad_variant_access default ctor must be noexcept";
  EXPECT_TRUE(noexcept(std::bad_variant_access{}.what())) << "bad_variant_access::what() must be noexcept";

  std::bad_variant_access exception;
  EXPECT_NE(exception.what(), nullptr) << "A message must be returned";
}