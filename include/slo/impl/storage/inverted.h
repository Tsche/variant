#pragma once
#include <cstddef>
#include <utility>
#include <memory>

#include <slo/util/list.h>
#include <slo/impl/union/recursive.h>
#include <slo/impl/union/tree.h>
#include "common.h"

namespace slo::impl {
template <auto Idx, typename ValueType>
struct TaggedWrapper {
  using type = ValueType;

  template <typename... Args>
  constexpr explicit TaggedWrapper(Args&&... args) : tag{Idx}
                                                   , storage{std::forward<Args>(args)...} {}

  [[nodiscard]] constexpr std::size_t index() const { return tag; }

  [[nodiscard]] constexpr ValueType& unwrap() & { return storage; }
  [[nodiscard]] constexpr ValueType const& unwrap() const& { return storage; }
  [[nodiscard]] constexpr ValueType&& unwrap() && { return std::move(storage); }
  [[nodiscard]] constexpr ValueType const&& unwrap() const&& { return std::move(storage); }

  decltype(Idx) tag;
  ValueType storage;
};

template <typename... Ts>
union InvertedStorage {
  // inverted variant
  using alternatives               = util::TypeList<Ts...>;
  using index_type                 = alternatives::index_type;
  constexpr static index_type npos = static_cast<index_type>(~0U);

  template <std::size_t... Is>
  static constexpr auto generate_union(std::index_sequence<Is...>)
      -> RecursiveUnion<TaggedWrapper<static_cast<index_type>(Is), Ts>...>;

  template <std::size_t... Is>
    requires(sizeof...(Ts) >= 42)
  static constexpr auto generate_union(std::index_sequence<Is...>)
      -> util::to_cbt<TreeUnion, TaggedWrapper<static_cast<index_type>(Is), Ts>...>;

  using union_type = decltype(generate_union(std::index_sequence_for<Ts...>()));

  TaggedWrapper<npos, error_type> dummy{};
  struct Container {
    union_type value;
  } storage;

public:
  template <std::size_t Idx, typename... Args>
  constexpr explicit InvertedStorage(std::in_place_index_t<Idx> idx, Args&&... args)
      : storage{union_type(idx, std::forward<Args>(args)...)} {}

  constexpr InvertedStorage() : dummy() {}
  constexpr ~InvertedStorage() { reset(); }

  constexpr InvertedStorage(InvertedStorage const& other) {
    slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
  }
  constexpr InvertedStorage(InvertedStorage&& other) noexcept {
    slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::move(obj)); }, std::move(other));
  }
  InvertedStorage& operator=(InvertedStorage const& other) {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T const& obj) { this->emplace<T>(obj); }, other);
    }
    return *this;
  }
  InvertedStorage& operator=(InvertedStorage&& other) noexcept {
    if (this != std::addressof(other)) {
      slo::visit([this]<typename T>(T&& obj) { this->emplace<T>(std::move(obj)); }, std::move(other));
    }
    return *this;
  }

  [[nodiscard]] constexpr std::size_t index() const {
    if consteval {
      if (!compat::is_within_lifetime(&dummy.tag)) {
        return storage.value.index();
      }
    }
    return dummy.index();
  }

  constexpr void reset() {
    if (index() != npos) {
      slo::visit([](auto&& member) { std::destroy_at(std::addressof(member)); }, *this);
      std::construct_at(&dummy);
    }
  }

  template <std::size_t Idx, typename... Args>
  constexpr void emplace(Args&&... args) {
    reset();
    std::construct_at(&storage.value, std::in_place_index<Idx>, std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  constexpr void emplace(Args&&... args) {
    emplace<alternatives::template get_index<T>>(std::forward<Args>(args)...);
  }

  template <std::size_t Idx, typename Self>
  constexpr decltype(auto) get(this Self&& self) {
    return std::forward<Self>(self).storage.value.template get<Idx>();
  }
};

}  // namespace slo::impl