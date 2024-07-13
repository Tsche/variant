#include <gtest/gtest.h>
#include <slo/util/compat.h>
#include <common/assertions.h>
struct In;
struct Out;
#define CHECK(T, U, R) ASSERT_SAME((slo::compat::like_t<T, U>), R)

// P2445R1  8.1 and 8.2 "merge" model
// clang-format off

TEST(ForwardLike, Value){
  CHECK(In,         Out,         Out&&);        // 8.1) 1
  CHECK(In,         Out&,        Out&&);        // 8.2) 19
  CHECK(In,         Out&&,       Out&&);        // 8.2) 24
  CHECK(In,         Out const,   Out const&&);  // 8.1) 7
  CHECK(In,         Out const&,  Out const&&);  // 8.2) 29
  CHECK(In,         Out const&&, Out const&&);  // 8.2) 33
}

TEST(ForwardLike, Lvref){
  CHECK(In&,        Out,         Out&);         // 8.1) 2
  CHECK(In&,        Out&,        Out&);         // 8.1) 13
  CHECK(In&,        Out&&,       Out&);         // 8.1) 14
  CHECK(In&,        Out const,   Out const&);   // 8.1) 8
  CHECK(In&,        Out const&,  Out const&);   // 8.1) 15
  CHECK(In&,        Out const&&, Out const&);   // 8.1) 16
}

TEST(ForwardLike, Rvref){
  CHECK(In&&,       Out,         Out&&);        // 8.1) 3
  CHECK(In&&,       Out&,        Out&&);        // 8.2) 20
  CHECK(In&&,       Out&&,       Out&&);        // 8.2) 25
  CHECK(In&&,       Out const,   Out const&&);  // 8.1) 9
  CHECK(In&&,       Out const&,  Out const&&);  // 8.2) 30
  CHECK(In&&,       Out const&&, Out const&&);  // 8.2) 34
}

TEST(ForwardLike, ConstValue){
  CHECK(In const,   Out,         Out const&&);  // 8.1) 4
  CHECK(In const,   Out&,        Out const&&);  // 8.2) 21
  CHECK(In const,   Out&&,       Out const&&);  // 8.2) 26
  CHECK(In const,   Out const,   Out const&&);  // 8.1) 10
  CHECK(In const,   Out const&,  Out const&&);  // 8.2) 31
  CHECK(In const,   Out const&&, Out const&&);  // 8.2) 35
}

TEST(ForwardLike, ConstLvref){
  CHECK(In const&,  Out,         Out const&);   // 8.1) 5
  CHECK(In const&,  Out&,        Out const&);   // 8.2) 22
  CHECK(In const&,  Out&&,       Out const&);   // 8.2) 27
  CHECK(In const&,  Out const,   Out const&);   // 8.1) 11
  CHECK(In const&,  Out const&,  Out const&);   // 8.1) 17
  CHECK(In const&,  Out const&&, Out const&);   // 8.1) 18
}

TEST(ForwardLike, ConstRvref){
  CHECK(In const&&, Out,         Out const&&);  // 8.1) 6
  CHECK(In const&&, Out&,        Out const&&);  // 8.2) 23
  CHECK(In const&&, Out&&,       Out const&&);  // 8.2) 28
  CHECK(In const&&, Out const,   Out const&&);  // 8.1) 12
  CHECK(In const&&, Out const&,  Out const&&);  // 8.2) 32
  CHECK(In const&&, Out const&&, Out const&&);  // 8.2) 36
}

// clang-format on
#undef CHECK