#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <type_traits>
#include <string>
#include <memory>
#include <slo/variant.h>

enum State { DELETED, DEFINED, TRIVIAL };

struct Traits {
  State destructor;
  State copy_constructor;
  State move_constructor;
  State copy_assignment;
  State move_assignment;

  template <typename T>
  void verify() const {
    ASSERT_EQ(std::is_destructible_v<T>, destructor != DELETED);
    ASSERT_EQ(std::is_trivially_destructible_v<T>, destructor == TRIVIAL);
    ASSERT_EQ(std::is_copy_constructible_v<T>, copy_constructor != DELETED);
    ASSERT_EQ(std::is_trivially_copy_constructible_v<T>, copy_constructor == TRIVIAL);
    ASSERT_EQ(std::is_move_constructible_v<T>, move_constructor != DELETED);
    ASSERT_EQ(std::is_trivially_move_constructible_v<T>, move_constructor == TRIVIAL);
    ASSERT_EQ(std::is_copy_assignable_v<T>, copy_assignment != DELETED);
    ASSERT_EQ(std::is_trivially_copy_assignable_v<T>, copy_assignment == TRIVIAL);
    ASSERT_EQ(std::is_move_assignable_v<T>, move_assignment != DELETED);
    ASSERT_EQ(std::is_trivially_move_assignable_v<T>, move_assignment == TRIVIAL);
  }
};


union Trivial {
  int foo;
  char bar;
};

TEST(SpecialMembers, Trivial) {
  using standard = std::variant<int, char>;
  using normal   = slo::NormalVariant<int, char>;
  using inverted = slo::InvertedVariant<int, char>;
  using proxy    = slo::Union<&Trivial::foo, &Trivial::bar>;

  auto expected = Traits{.destructor       = TRIVIAL,
                         .copy_constructor = TRIVIAL,
                         .move_constructor = TRIVIAL,
                         .copy_assignment  = TRIVIAL,
                         .move_assignment  = TRIVIAL};

  expected.verify<standard>();
  expected.verify<normal>();
  expected.verify<inverted>();
  expected.verify<proxy>();
}


union NonTrivial {
  int foo;
  std::string bar;
};

TEST(SpecialMembers, NonTrivial) {
  using standard = std::variant<int, std::string>;
  using normal   = slo::NormalVariant<int, std::string>;
  using inverted = slo::InvertedVariant<int, std::string>;
  using proxy    = slo::Union<&NonTrivial::foo, &NonTrivial::bar>;

  auto expected = Traits{.destructor       = DEFINED,
                         .copy_constructor = DEFINED,
                         .move_constructor = DEFINED,
                         .copy_assignment  = DEFINED,
                         .move_assignment  = DEFINED};

  expected.verify<standard>();
  expected.verify<normal>();
  expected.verify<inverted>();
  expected.verify<proxy>();
}


union MoveOnly {
  int foo;
  std::unique_ptr<int> bar;
};

TEST(SpecialMembers, MoveOnly) {
  using standard = std::variant<int, std::unique_ptr<int>>;
  using normal   = slo::NormalVariant<int, std::unique_ptr<int>>;
  using proxy    = slo::Union<&MoveOnly::foo, &MoveOnly::bar>;

  auto expected = Traits{.destructor       = DEFINED,
                         .copy_constructor = DELETED,
                         .move_constructor = DEFINED,
                         .copy_assignment  = DELETED,
                         .move_assignment  = DEFINED};

  expected.verify<standard>();
  expected.verify<normal>();
  expected.verify<proxy>();
}