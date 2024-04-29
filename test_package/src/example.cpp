#include <slo/variant.h>
#include <iostream>


int main() {
  auto variant = slo::variant<int, char, float>{3.2F};
  slo::visit([](auto obj) {
    std::cout << obj << '\n';
  }, variant);
}