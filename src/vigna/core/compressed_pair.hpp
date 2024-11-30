//
// Created by Ninter6 on 2024/11/24.
//

#pragma once

#include <utility>

namespace vigna {
template <class First, class Second>
struct basic_compressed_pair : First, Second {
    basic_compressed_pair() = default;
    basic_compressed_pair(First&& f, Second&& s) : First(std::forward<First>(f)), Second(std::forward<Second>(s)) {}

    [[nodiscard]] First& first() { return *this; }
    [[nodiscard]] const First& first() const { return *this; }
    [[nodiscard]] Second& second() { return *this; }
    [[nodiscard]] const Second& second() const { return *this; }
};

template <class First, class Second>
struct compressed_pair : private basic_compressed_pair<First, Second> {
    using basic_compressed_pair<First, Second>::basic_compressed_pair;
    using basic_compressed_pair<First, Second>::first;
    using basic_compressed_pair<First, Second>::second;
};
} // namespace vigna
