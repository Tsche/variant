#include <type_traits>
#include <utility>
#include <slo/util/list.h>

#if defined(TREE) or defined(RECURSIVE)
#  include <slo/variant.h>
#else
#  include <variant>
#endif

template <auto>
struct Constant {};

template <std::size_t... Idx>
auto generate_union(std::index_sequence<Idx...>) {
#if defined(TREE)
  using underlying = slo::util::to_cbt<slo::impl::TreeUnion, Constant<Idx>...>;
#elif defined(RECURSIVE)
  using underlying = slo::impl::RecursiveUnion<Constant<Idx>...>;
#else
  using underlying = std::variant<Constant<Idx>...>;
#endif
  return std::type_identity<underlying>{};
}

template <typename T>
auto get_size() {
#if defined(TREE)
  return T::size;
#else
  return 0;
#endif
}

template <typename T, std::size_t... Idx>
auto generate_getters(std::index_sequence<Idx...>) {
  (
      [] {
        auto obj = T{std::in_place_index<Idx>};
#if defined(TREE) || defined(RECURSIVE)
        using slo::get;
#else
        using std::get;
#endif
        (void)get<Idx>(obj);
      }(),
      ...);
}

int main() {
  using TestType = typename decltype(generate_union(std::make_index_sequence<200>()))::type;
  //(void)get_size<TestType>();

  generate_getters<TestType>(std::make_index_sequence<200>());
}
