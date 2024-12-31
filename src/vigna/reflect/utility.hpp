//
// Created by Ninter6 on 2024/12/12.
//

#pragma once

#include <type_traits>

namespace vigna::reflect {

namespace detail {

template <size_t I, class...Args>
struct type_list_element_helper;

template <size_t I, class First, class...Other>
struct type_list_element_helper<I, First, Other...>
    : type_list_element_helper<I - 1, Other...> {};

template <class T, class...Other>
struct type_list_element_helper<0u, T, Other...> {
    using type = T;
};

template <size_t I, class T, class...Args>
struct type_list_find_helper;

template <size_t I, class T, class First, class...Args>
struct type_list_find_helper<I, T, First, Args...>
    : type_list_find_helper<I + 1, T, Args...> {};

template <size_t I, class T, class...Args>
struct type_list_find_helper<I, T, T, Args...> {
    static constexpr size_t value = I;
};

template <size_t I, class T>
struct type_list_find_helper<I, T> {
    static constexpr size_t value = I;
};

}

template <class...Args>
struct type_list {
    using type = type_list;
    static constexpr auto size = sizeof...(Args);
};

template <size_t I, class List>
struct type_list_element;

template <size_t I, template<class...> class List, class...Args>
struct type_list_element<I, List<Args...>> {
    using type = typename detail::type_list_element_helper<I, Args...>::type;
};

template <size_t I, class List>
using type_list_element_t = typename type_list_element<I, List>::type;

template <class T, class List>
struct type_list_find;

template <class T, template<class...> class List, class...Args>
struct type_list_find<T, List<Args...>>
    : detail::type_list_find_helper<0, T, Args...> {};

template <class T, class List>
constexpr size_t type_list_find_v = type_list_find<T, List>::value;

template <class To, class From>
using constness_as_t = std::conditional_t<std::is_const_v<From>,
    std::add_const_t<To>,
    std::remove_const_t<To>>;

}
