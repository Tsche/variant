#pragma once

template <auto V>
struct Constant {
  static constexpr auto value = V;
};

template <auto V>
inline constexpr Constant<V> constant;