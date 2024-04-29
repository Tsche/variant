#pragma once
#include <cstddef>
#include <utility>

#include "pack_generated.h"


namespace slo::util::pack {
template <std::size_t Idx, typename T>
struct GetImpl;

template <std::size_t Idx, template <typename...> class List, typename... Ts>
  requires(Idx > MAX_SPEC)
struct GetImpl<Idx, List<Ts...>> {
#if __has_builtin(__type_pack_element)
  using type = __type_pack_element<Idx, Ts...>;

  template <std::size_t offset, std::size_t... indices>
  constexpr static auto do_slice(std::index_sequence<indices...>) noexcept
      -> List<__type_pack_element<indices + offset, Ts...>...>;

  using tail = decltype(do_slice<Idx + 1>(std::make_index_sequence<sizeof...(Ts) - (Idx + 1)>{}));
#else
  using first_page = GetImpl<MAX_SPEC, List<Ts...>>;
  using recurse    = GetImpl<Idx - (MAX_SPEC + 1), typename first_page::tail>;

  using type = typename recurse::type;
  using tail = typename recurse::tail;
#endif
};

template <std::size_t Idx, typename T>
using get = GetImpl<Idx, T>::type;
}  // namespace librepr::pack