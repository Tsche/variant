#include <gtest/gtest.h>
#include <slo/util/compat.h>

struct In;
struct Out;
#define CHECK(T, U, R) ASSERT_TRUE((std::is_same_v<slo::compat::like_t<T, U>, R>))

// P2445R1  8.1 and 8.2 "merge" model
// clang-format off

TEST(forward_like, value){
  CHECK(In,         Out,         Out&&);        // 8.1) 1
  CHECK(In,         Out&,        Out&&);        // 8.2) 19
  CHECK(In,         Out&&,       Out&&);        // 8.2) 24
  CHECK(In,         Out const,   Out const&&);  // 8.1) 7
  CHECK(In,         Out const&,  Out const&&);  // 8.2) 29
  CHECK(In,         Out const&&, Out const&&);  // 8.2) 33
}

TEST(forward_like, lvref){
  CHECK(In&,        Out,         Out&);         // 8.1) 2
  CHECK(In&,        Out&,        Out&);         // 8.1) 13
  CHECK(In&,        Out&&,       Out&);         // 8.1) 14
  CHECK(In&,        Out const,   Out const&);   // 8.1) 8
  CHECK(In&,        Out const&,  Out const&);   // 8.1) 15
  CHECK(In&,        Out const&&, Out const&);   // 8.1) 16
}

TEST(forward_like, rvref){
  CHECK(In&&,       Out,         Out&&);        // 8.1) 3
  CHECK(In&&,       Out&,        Out&&);        // 8.2) 20
  CHECK(In&&,       Out&&,       Out&&);        // 8.2) 25
  CHECK(In&&,       Out const,   Out const&&);  // 8.1) 9
  CHECK(In&&,       Out const&,  Out const&&);  // 8.2) 30
  CHECK(In&&,       Out const&&, Out const&&);  // 8.2) 34
}

TEST(forward_like, const_value){
  CHECK(In const,   Out,         Out const&&);  // 8.1) 4
  CHECK(In const,   Out&,        Out const&&);  // 8.2) 21
  CHECK(In const,   Out&&,       Out const&&);  // 8.2) 26
  CHECK(In const,   Out const,   Out const&&);  // 8.1) 10
  CHECK(In const,   Out const&,  Out const&&);  // 8.2) 31
  CHECK(In const,   Out const&&, Out const&&);  // 8.2) 35
}

TEST(forward_like, const_lvref){
  CHECK(In const&,  Out,         Out const&);   // 8.1) 5
  CHECK(In const&,  Out&,        Out const&);   // 8.2) 22
  CHECK(In const&,  Out&&,       Out const&);   // 8.2) 27
  CHECK(In const&,  Out const,   Out const&);   // 8.1) 11
  CHECK(In const&,  Out const&,  Out const&);   // 8.1) 17
  CHECK(In const&,  Out const&&, Out const&);   // 8.1) 18
}

TEST(forward_like, const_rvref){
  CHECK(In const&&, Out,         Out const&&);  // 8.1) 6
  CHECK(In const&&, Out&,        Out const&&);  // 8.2) 23
  CHECK(In const&&, Out&&,       Out const&&);  // 8.2) 28
  CHECK(In const&&, Out const,   Out const&&);  // 8.1) 12
  CHECK(In const&&, Out const&,  Out const&&);  // 8.2) 32
  CHECK(In const&&, Out const&&, Out const&&);  // 8.2) 36
}

// clang-format on
#undef CHECK