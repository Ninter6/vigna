//
// Created by Ninter6 on 2024/12/19.
//

#pragma once

#include <any>

#include "signal.hpp"

namespace vigna {

template <class Alloc = std::allocator<void>>
class emitter {
    using alloc_traits = std::allocator_traits<Alloc>;

    using key_type = std::type_info;
    using value_type = std::any;
    using container = dense_map<key_type, value_type,
        std::hash<key_type>,
        std::equal_to<key_type>,
        typename alloc_traits::template rebind_alloc<std::pair<const key_type, value_type>>>;

public:
    emitter() = default;

    template <class ... Args>
    connection connect() {

    }

private:
    container signals_;

};

}
