#pragma once

#include <sol.hpp>

#include <tuple>

#ifndef EMBER_REFLECTION_MAX_TYPES
#define EMBER_REFLECTION_MAX_TYPES 32
#endif

namespace component {

void register_all_components(sol::table& table);

// Type registry
template <std::size_t N>
struct _type_registry_index : _type_registry_index<N - 1> {};
template <>
struct _type_registry_index<0> {};
inline std::tuple<> _type_registry(_type_registry_index<0>) { return {}; }
inline void _type_registry(_type_registry_index<EMBER_REFLECTION_MAX_TYPES + 1>) = delete;

} // namespace component
