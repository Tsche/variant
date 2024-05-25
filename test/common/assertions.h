#pragma once
#include "type_name.h"

#define DEPAREN(X) ESC(ISH X)
#define ISH(...)   ISH __VA_ARGS__
#define ESC(...)   ESC_(__VA_ARGS__)
#define ESC_(...)  VAN##__VA_ARGS__
#define VANISH

#define EXPECT_SAME(first, second)                                                                                 \
  EXPECT_TRUE((std::is_same_v<DEPAREN(first), DEPAREN(second)>)) << "  Actual: " << nameof<DEPAREN(first)> << '\n' \
                                                                 << "Expected: " << nameof<DEPAREN(second)>

#define EXPECT_NOEXCEPT(...) EXPECT_TRUE((noexcept(__VA_ARGS__))) << "Operation must be noexcept"