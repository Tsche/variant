#include <gtest/gtest.h>
#include <slo/variant.h>


struct Incomplete;
template <typename T>
struct Wrapper { T obj; };

TEST(Visit, Incomplete){
  auto obj = slo::Variant<Wrapper<Incomplete>*, int>{};

  slo::visit([](auto){}, obj);
  slo::visit([](auto) -> Wrapper<Incomplete>* { return nullptr; }, obj);
  // slo::visit<void>([](auto){}, obj);
  // slo::visit<void*>([](auto) -> Wrapper<Incomplete>* { return nullptr; }, obj);
}