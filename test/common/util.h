#pragma once
#include <gtest/gtest.h>
#include <slo/variant.h>

template <auto V>
struct Constant {
  static constexpr auto value = V;
};


template <template <typename...> class Variant>
struct VariantWrapper {
  template <typename... Ts>
  using type = Variant<Ts...>;
};

using test_variants = testing::Types<VariantWrapper<slo::NormalVariant>,
                                     VariantWrapper<slo::RecursiveVariant>,
                                     VariantWrapper<slo::TreeVariant>,

                                     VariantWrapper<slo::InvertedVariant>,
                                     VariantWrapper<slo::InvertedRecursiveVariant>,
                                     VariantWrapper<slo::InvertedTreeVariant>,

                                     VariantWrapper<slo::Variant> >;

template <typename T, typename... Ts>
using get_variant = typename T::template type<Ts...>;