#include <cstddef>

#if TIERED == 1
#include "strategy.h"
#else
#include "simple.h"
#endif

struct Test {
  constexpr static std::size_t size = TEST_SIZE;
  std::size_t tag                   = 0;
  [[nodiscard]] std::size_t index() const noexcept { return tag; }
};

int main(int argc, char** argv) {
  auto obj = Test(argc);
  visit(obj);
}