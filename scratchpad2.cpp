#include <slo/union.h>

union Union {
  enum class Tag : unsigned char { integer, character, floating, uint };

  int integer;
  char character;
  float floating;
  unsigned uint;

  using Tagged = slo::Union<{Tag::integer, &Union::integer},
                            {Tag::character, &Union::character},
                            {Tag::floating, &Union::floating},
                            {Tag::uint, &Union::uint}>;
};

int main() {
  Union::Tagged variant;
  variant.value.*variant.get_ptr<0>();
}