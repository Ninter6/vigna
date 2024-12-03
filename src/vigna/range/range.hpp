//
// Created by Ninter6 on 2024/12/2.
//

#pragma once

#include <utility>

namespace vigna::range {

using std::begin;
using std::end;

template <class T, class = void>
constexpr bool is_range_v = false;
template <class T>
constexpr bool is_range_v<T, std::void_t<decltype(begin(std::declval<T>()), end(std::declval<T>()))>> = true;

template <class T>
using is_range_t = std::enable_if_t<is_range_v<T>>;

template <class T, class = is_range_t<T>>
constexpr size_t size(T&& rg) {
    if constexpr (requires { rg.size(); })
        return rg.size();
    else
        return std::distance(begin(rg), end(rg));
}

}
