#pragma once

template <auto V>
struct Constant {
  static constexpr auto value = V;
};