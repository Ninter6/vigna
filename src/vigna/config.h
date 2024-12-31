//
// Created by Ninter6 on 2024/11/24.
//

#pragma once

#include <cstdint>

#define VIGNA_SPARSE_PAGE 4096
#define VIGNA_ENTITY_TYPE uint32_t

#ifndef VIGNA_NO_ETO // empty type optimization
#   define VIGNA_ETO(x) std::enable_if_t< std::is_empty_v<x> >
#else
#   define VIGNA_ETO(x) std::enable_if_t<(1+1 == 9)>
#endif

#ifndef VIGNA_NO_SIGNAL_MIXIN
#   define VIGNA_MIXIN(mixin, ...) mixin<__VA_ARGS__>
#else
#   define VIGNA_MIXIN(mixin, storage, ...) storage
#endif

#if __cplusplus >= 202002L && !defined(VIGNA_NO_SOURCE_LOCATION)
#   include <source_location>
#endif
#if __cpp_lib_source_location >= 201907L
#   define VIGNA_PRETTY_FUNCTION (std::source_location::current().function_name())
#elif !defined(VIGNA_NO_PRETTY_FUNCTION)
#   if defined(__GNUC__) || defined(__clang__)
#       define VIGNA_PRETTY_FUNCTION __PRETTY_FUNCTION__
#   elif defined(_MSC_VER)
#       define VIGNA_PRETTY_FUNCTION __FUNCSIG__
#   endif
#endif

#ifdef VIGNA_PRETTY_FUNCTION
#   ifdef _MSC_VER
#       define VIGNA_PRETTY_FUNCTION_PREFIX '<'
#       define VIGNA_PRETTY_FUNCTION_SUFFIX '>'
#   else
#       define VIGNA_PRETTY_FUNCTION_PREFIX '='
#       define VIGNA_PRETTY_FUNCTION_SUFFIX ']'
#   endif
#endif
