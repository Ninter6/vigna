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

template <class T, class R = void>
using is_range_t = std::enable_if_t<is_range_v<T>, R>;

template <class, class = void>
constexpr bool is_iterable_v = false;
template <class T>
constexpr bool is_iterable_v<T, std::enable_if_t<
    std::is_copy_constructible_v<T>,
    std::void_t<decltype(*++std::declval<T>(),
                         std::declval<T>() != std::declval<T>())>>> = true;

template <class, class, class = void>
constexpr bool is_compatible_iterator_v = false;
template <class T, class U>
constexpr bool is_compatible_iterator_v<T, U, std::enable_if_t<
    is_iterable_v<T> && is_iterable_v<U>,
    std::void_t<decltype(std::declval<T>() != std::declval<U>())>>> = true;

struct identity {
    template <class U>
    constexpr decltype(auto) operator()(U&& u) const {
        return std::forward<U>(u);
    }
};

}
