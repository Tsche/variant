#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include "slo/util/list.h"
#include "slo/util/pack.h"

namespace slo {

template <typename K, typename V>
struct UnionMember;

template <typename K, typename V, typename C>
  requires std::is_union_v<C>
struct UnionMember<K, V C::*> {
  using key_type   = K;
  using class_type = C;
  using type       = V;

  K key;
  V C::* value;
};

template <typename K, typename V, typename C>
UnionMember(K, V C::*) -> UnionMember<K, V C::*>;

template <typename...>
struct MemberList;

template <typename K, typename U, typename... Ts>
  requires std::is_union_v<U>
struct MemberList<UnionMember<K, Ts U::*>...> {
  using alternatives = util::TypeList<Ts...>;
  using union_type   = U;
  using key_type     = K;
};

template<auto Value>
struct Wrap {
  static constexpr decltype(Value) value = Value;
};

template <UnionMember... Members>
struct Union {
  using members = MemberList<decltype(Members)...>;
  static constexpr members::key_type npos = static_cast<members::key_type>(~0ULL);
  members::key_type key;
  members::union_type value;

  template <std::size_t Idx>
  static constexpr auto get_ptr() {
    using member_ptrs = util::TypeList<Wrap<Members>...>;
    return util::pack::get<Idx, member_ptrs>::value.member_ptr;
  }

  constexpr void reset() {
    if (key != npos){
      slo::visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
      key = npos;
    }
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&value.*get_ptr<Idx>(), std::forward<Args>(args)...);
    // set key
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).value.*get_ptr<Idx>();
  }
};

}  // namespace slo