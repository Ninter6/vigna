//
// Created by Ninter6 on 2025/1/1.
//

#pragma once

#include <algorithm>

#include "range.hpp"
#include "view.hpp"

namespace vigna::range {

constexpr struct for_each_t {
    template <class R, class F, class P = identity>
    constexpr void operator()(R&& range, F&& func, P&& proj = {}) const {
        auto v = std::forward<R>(range) | view::transform(std::forward<P>(proj)) | view::common;
        std::for_each(begin(v), end(v), std::forward<F>(func));
    }
} for_each{};

constexpr struct for_each_n_t {
    template <class R, class F, class P = identity>
    constexpr void operator()(R&& range, size_t n, F&& func, P&& proj = {}) const {
        auto v = std::forward<R>(range) | view::transform(std::forward<P>(proj)) | view::common;
        std::for_each_n(begin(v), n, std::forward<F>(func));
    }
} for_each_n{};

}
