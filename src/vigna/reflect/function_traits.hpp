//
// Created by Ninter6 on 2024/12/12.
//

#pragma once

#include "utility.hpp"

namespace vigna::reflect {

namespace detail {

template <class F>
struct function_traits {};

template <class R, class...Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using arguments = type_list<Args...>;

    constexpr bool is_member_function_pointer = false;
};

template <class R, class C, class...Args>
struct function_traits<R(C::*)(Args...)> {
    using return_type = R;
    using arguments = type_list<Args...>;

    using clazz = C;
    constexpr bool is_member_function_pointer = true;
};

}

using detail::function_traits;

}
