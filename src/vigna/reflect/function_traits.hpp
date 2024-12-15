//
// Created by Ninter6 on 2024/12/12.
//

#pragma once

#include "utility.hpp"

namespace vigna::reflect {

namespace detail {

template <class F>
struct function_traits {};

template <class R, class...Args, bool Ne>
struct function_traits<R(Args...) noexcept(Ne)> {
    using type = R(Args...) noexcept(Ne);

    using return_type = R;
    using arguments = type_list<Args...>;
    using sign = R(Args...);

    static constexpr auto arg_count = arguments::size;

    static constexpr bool is_noexcept = Ne;

    static constexpr bool is_member_pointer = false;
};

template <class R, class C, class...Args, bool Ne>
struct function_traits<R(C::*)(Args...) noexcept(Ne)> {
    using type = R(C::*)(Args...) noexcept(Ne);

    using return_type = R;
    using arguments = type_list<Args...>;
    using sign = R(Args...);

    static constexpr auto arg_count = arguments::size;

    static constexpr bool is_noexcept = Ne;

    using clazz = C;
    static constexpr bool is_member_pointer = true;
    static constexpr bool is_const = false;

};

template <class R, class C, class...Args, bool Ne>
struct function_traits<R(C::*)(Args...) const noexcept(Ne)> {
    using type = R(C::*)(Args...) const noexcept(Ne);

    using return_type = R;
    using arguments = type_list<Args...>;
    using sign = R(Args...);

    static constexpr auto arg_count = arguments::size;

    static constexpr bool is_noexcept = Ne;

    using clazz = C;
    static constexpr bool is_member_pointer = true;
    static constexpr bool is_const = true;

};

}

using detail::function_traits;

}
