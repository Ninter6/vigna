//
// Created by Ninter6 on 2024/12/12.
//

#pragma once

#include <type_traits>

namespace vigna::reflect {

template<class...Args>
struct type_list {
    using type = type_list;
    constexpr auto size = sizeof...(Args);
};

template<size_t I, class...Args>
struct type_list_element;

template<size_t I, class First, class...Other>
struct type_list_element<I, First, Other...>
    : type_list_element<I - 1, Other...> {};

template<class T>
struct type_list_element<0u, T> {
    using type = T;
};

template<size_t I>
using type_list_element_t = typename type_list_element<I>::type;

}
