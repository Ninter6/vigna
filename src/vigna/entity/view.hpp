//
// Created by Ninter6 on 2024/12/31.
//

#pragma once

#include "vigna/range/view.hpp"
#include "vigna/reflect/utility.hpp"

namespace vigna {

template <class...Args>
using get_t = reflect::type_list<Args...>;

template <class...Args>
using exclude_t = reflect::type_list<Args...>;

template <class, class>
class basic_view;

}
