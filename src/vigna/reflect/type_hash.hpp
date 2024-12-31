//
// Created by Ninter6 on 2024/12/30.
//

#pragma once

#include <string_view>

#include "vigna/config.h"

namespace vigna::reflect {

namespace detail {
static inline size_t type_index_curr = 0;
}

template <class>
auto type_index() {
    static const auto index = detail::type_index_curr++;
    return index;
}

template <class T>
constexpr auto type_name() {
#ifdef VIGNA_PRETTY_FUNCTION
    auto str = std::string_view{VIGNA_PRETTY_FUNCTION};
    auto first = str.find_first_not_of(' ', str.find(VIGNA_PRETTY_FUNCTION_PREFIX) + 1);
    return str.substr(first, str.rfind(VIGNA_PRETTY_FUNCTION_SUFFIX) - first);
#else
    return std::string_view{""};
#endif
}

template <class T, class HashValue = size_t>
constexpr HashValue type_hash() {
    auto name = type_name<T>();
    size_t hash_value = 14695981039346656037u;

    if (name.empty())
        return static_cast<HashValue>(type_index<T>());

    for (auto&& c : name) {
        constexpr size_t prime2 = 0x5deece66d;
        constexpr size_t prime1 = 0x9e3779b9;
        hash_value ^= c;
        hash_value *= prime1;
        hash_value = (hash_value << 31) | (hash_value >> (sizeof(hash_value) * 8 - 31));
        hash_value *= prime2;
    }

    return static_cast<HashValue>(hash_value & ~HashValue(0));
}

}
