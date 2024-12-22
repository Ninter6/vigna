//
// Created by Ninter6 on 2024/12/12.
//

#pragma once

#include <type_traits>

namespace vigna::reflect {

namespace detail {

template<size_t I, class...Args>
struct type_list_element_helper;

template<size_t I, class First, class...Other>
struct type_list_element_helper<I, First, Other...>
    : type_list_element_helper<I - 1, Other...> {};

template<class T, class...Other>
struct type_list_element_helper<0u, T, Other...> {
    using type = T;
};

}

template<class...Args>
struct type_list {
    using type = type_list;
    static constexpr auto size = sizeof...(Args);
};

template<size_t I, class List>
struct type_list_element;

template<size_t I, template<class...> class List, class...Args>
struct type_list_element<I, List<Args...>> {
    using type = typename detail::type_list_element_helper<I, Args...>::type;
};

template<size_t I, class List>
using type_list_element_t = typename type_list_element<I, List>::type;

template <class To, class From>
using constness_as_t = std::conditional_t<std::is_const_v<From>,
    std::add_const_t<To>,
    std::remove_const_t<To>>;

}
