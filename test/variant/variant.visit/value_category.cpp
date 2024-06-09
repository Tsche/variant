#include <gtest/gtest.h>
#include <slo/variant.h>
#include <common/assertions.h>

#include "forwarding_visitor.h"

TEST(CallOperatorForwarding, NoVariant) {
  auto visitor              = ForwardingTestVisitor{};
  auto const& const_visitor = visitor;

  auto result0 = slo::visit(visitor);
  result0.verify<>({Qualifiers::CV::None, Qualifiers::Lvalue});

  auto result1 = slo::visit(const_visitor);
  result1.verify<>({Qualifiers::Const, Qualifiers::Lvalue});

  auto result2 = slo::visit(std::move(visitor));
  result2.verify<>({Qualifiers::CV::None, Qualifiers::Rvalue});

  auto result3 = slo::visit(std::move(const_visitor));
  result3.verify<>({Qualifiers::Const, Qualifiers::Rvalue});
}

TEST(CallOperatorForwarding, SingleVariant) {
  auto visitor              = ForwardingTestVisitor{};
  auto const& const_visitor = visitor;

  auto obj = slo::Variant<int, char, float>{42};

  auto result0 = slo::visit(visitor, obj);
  result0.verify<int&>({Qualifiers::CV::None, Qualifiers::Lvalue});

  auto result1 = slo::visit(const_visitor, obj);
  result1.verify<int&>({Qualifiers::Const, Qualifiers::Lvalue});

  auto result2 = slo::visit(std::move(visitor), obj);
  result2.verify<int&>({Qualifiers::CV::None, Qualifiers::Rvalue});

  auto result3 = slo::visit(std::move(const_visitor), obj);
  result3.verify<int&>({Qualifiers::Const, Qualifiers::Rvalue});
}

TEST(CallOperatorForwarding, MultiVariant) {
  auto visitor              = ForwardingTestVisitor{};
  auto const& const_visitor = visitor;

  auto obj  = slo::Variant<int, char, float>{42};
  auto obj2 = slo::Variant<float, double>{42.F};

  auto result0 = slo::visit(visitor, obj, obj2);
  result0.verify<int&, float&>({Qualifiers::CV::None, Qualifiers::Lvalue});

  auto result1 = slo::visit(const_visitor, obj, obj2);
  result1.verify<int&, float&>({Qualifiers::Const, Qualifiers::Lvalue});

  auto result2 = slo::visit(std::move(visitor), obj, obj2);
  result2.verify<int&, float&>({Qualifiers::CV::None, Qualifiers::Rvalue});

  auto result3 = slo::visit(std::move(const_visitor), obj, obj2);
  result3.verify<int&, float&>({Qualifiers::Const, Qualifiers::Rvalue});
}

TEST(ArgumentForwarding, SingleVariant) {
  auto visitor = ForwardingTestVisitor{};

  auto obj              = slo::Variant<int, std::string, float>{42};
  auto const& const_obj = obj;

  auto result0 = slo::visit(visitor, obj);
  result0.verify<int&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  auto result1 = slo::visit(visitor, const_obj);
  result1.verify<int const&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  auto result2 = slo::visit(visitor, std::move(obj));
  result2.verify<int&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  auto result3 = slo::visit(visitor, std::move(const_obj));
  result3.verify<int const&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
}

TEST(ArgumentForwarding, MultiVariant) {
  auto visitor = ForwardingTestVisitor{};

  auto obj              = slo::Variant<int, std::string, float>{42};
  auto const& const_obj = obj;

  auto obj2              = slo::Variant<double, float>{42.0};
  auto const& const_obj2 = obj2;

  slo::visit(visitor, obj, obj2).verify<int&, double&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, const_obj, obj2).verify<int const&, double&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, obj, const_obj2).verify<int&, double const&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, const_obj, const_obj2)
      .verify<int const&, double const&>({Qualifiers::CV::None, Qualifiers::Lvalue});

  slo::visit(visitor, std::move(obj), obj2).verify<int&&, double&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, std::move(const_obj), obj2)
      .verify<int const&&, double&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, std::move(obj), const_obj2)
      .verify<int&&, double const&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, std::move(const_obj), const_obj2)
      .verify<int const&&, double const&>({Qualifiers::CV::None, Qualifiers::Lvalue});

  slo::visit(visitor, obj, std::move(obj2)).verify<int&, double&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, obj, std::move(const_obj2))
      .verify<int&, double const&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, const_obj, std::move(obj2))
      .verify<int const&, double&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, const_obj, std::move(const_obj2))
      .verify<int const&, double const&&>({Qualifiers::CV::None, Qualifiers::Lvalue});

  slo::visit(visitor, std::move(obj), std::move(obj2))
      .verify<int&&, double&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, std::move(const_obj), std::move(obj2))
      .verify<int const&&, double&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, std::move(obj), std::move(const_obj2))
      .verify<int&&, double const&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
  slo::visit(visitor, std::move(const_obj), std::move(const_obj2))
      .verify<int const&&, double const&&>({Qualifiers::CV::None, Qualifiers::Lvalue});
}

int dummy = 42;
struct ResultTestVisitor {
  int& operator()() & { return dummy; }
  int const& operator()() const& { return dummy; }
  int&& operator()() && { return std::move(dummy); }
  int const&& operator()() const&& { return std::move(std::as_const(dummy)); }
};

TEST(Visit, ReturnValueCategory) {
  auto visitor              = ResultTestVisitor{};
  auto const& const_visitor = visitor;
  auto&& result0            = slo::visit(visitor);
  EXPECT_SAME(decltype(result0), int&);

  auto&& result1 = slo::visit(const_visitor);
  EXPECT_SAME(decltype(result1), int const&);

  auto&& result2 = slo::visit(std::move(visitor));
  EXPECT_SAME(decltype(result2), int&&);

  auto&& result3 = slo::visit(std::move(const_visitor));
  EXPECT_SAME(decltype(result3), int const&&);
}