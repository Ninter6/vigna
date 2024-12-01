//
// Created by Ninter6 on 2024/12/1.
//

#pragma once

#include <utility>

namespace vigna {

template <class T>
struct input_iterator_pointer {
    static_assert(std::is_same_v<T, std::decay<T>>, "Invalid type!");

    using value_type = T;
    using pointer = T*;
    using reference = T&;

    input_iterator_pointer() = default;

    reference operator*() const {
        return inner;
    }

    pointer operator->() const {
        return &inner;
    }

    value_type inner;
};

}
