//
// Created by Ninter6 on 2024/12/2.
//

#pragma once

#include <functional>

#include "range.hpp"
#include "iterator.hpp"

namespace vigna::range {

template <class It, class Sentinel = It>
struct subrange {
    using value_type = std::decay_t<decltype(*std::declval<It>())>;
    using pointer = value_type*;
    using reference = value_type&;

    constexpr subrange() = default;
    constexpr subrange(It&& begin, Sentinel&& end)
    : begin_(std::forward<decltype(begin)>(begin)), end_(std::forward<decltype(end)>(end)) {}

    [[nodiscard]] constexpr decltype(auto) begin() const { return begin_; }
    [[nodiscard]] constexpr decltype(auto) end() const { return end_; }

private:
    It begin_;
    Sentinel end_;

};

namespace view {

struct view_factory {};

constexpr struct iota_t {
    template <class T = int, class = std::enable_if_t<std::is_integral_v<T>>>
    constexpr auto operator()(T n = T{0}) const {
        return subrange{iota_iterator<T>{n}, iota_iterator<T>{}};
    }
} iota{};

constexpr struct pack_t {
    template <class Range, class...Args, class = std::enable_if_t<(is_range_v<Args> && ...)>>
    constexpr auto operator()(Range&& rg, Args&&...args) const {
        return subrange{packed_iterator{begin(rg), begin(args)...}, packed_iterator{end(rg), end(args)...}};
    }
} pack{};

template <class Range, class T, class =
    std::enable_if_t<is_range_v<Range> && std::is_base_of_v<view_factory, std::decay_t<T>>>>
constexpr decltype(auto) operator|(Range&& rg, T&& fc) { return fc(std::forward<Range>(rg)); }

inline namespace _filter {

constexpr struct all_t : view_factory {
    constexpr auto& operator()() const {return *this;}
    template <class T, class = is_range_t<T>>
    constexpr auto operator()(T&& rg) const {
        return subrange{begin(rg), end(rg)};
    }
} all{};

constexpr struct common_t : view_factory {
    template <class T, class = is_range_t<T>>
    constexpr auto operator()(T&& rg) const {
        using It = std::decay_t<decltype(begin(rg))>;
        using Sen = std::decay_t<decltype(end(rg))>;
        if constexpr (std::is_same_v<It, Sen>) {
            return all(rg);
        } else {
            using common_it = common_iterator<It, Sen>;
            return subrange{common_it{begin(rg)}, common_it{end(rg)}};
        }
    }
} common{};

constexpr struct take_t {
    template <class T, class = void>
    struct impl_func;
    template <class T>
    struct impl_func<T, std::void_t<std::enable_if_t<is_range_v<T>, decltype(begin(std::declval<T>()) + size_t{})>>> {
        constexpr auto operator()(T&& rg, size_t n) const { return subrange{begin(rg), begin(rg) + n}; }
    };
    struct impl_obj : view_factory {
        template <class T, class = is_range_t<T>>
        constexpr auto operator()(T&& rg) const { return impl_func<T>{}(std::forward<T>(rg), n); }
        constexpr explicit impl_obj(size_t size) : n(size) {}
        size_t n;
    };
    template <class T, class = is_range_t<T>>
    constexpr auto operator()(T&& rg, size_t n) const { return impl_func<T>{}(std::forward<T>(rg), n); }
    constexpr auto operator()(size_t n) const { return impl_obj{n}; }
} take{};

constexpr struct reverse_t : view_factory {
    constexpr auto& operator()() const {return *this;}
    template <class T, class = std::void_t<is_range_t<T,
        decltype(std::make_reverse_iterator(end(std::declval<T>())))>>>
    auto operator()(T&& rg) const {
        return subrange{
            std::make_reverse_iterator(end(rg)),
            std::make_reverse_iterator(begin(rg)) };
    }
} reverse{};

constexpr struct filter_t {
    template <class Fn>
    struct impl_obj : view_factory {
        template <class T, class = std::enable_if_t<is_range_v<T> && std::is_invocable_r_v<bool, Fn, decltype(*begin(std::declval<T>()))>>>
        constexpr auto operator()(T&& rg) const {
            return subrange{filter_iterator{begin(rg), end(rg), fn}, filter_iterator{end(rg), end(rg), std::move(const_cast<impl_obj*>(this)->fn)}};
        }
        template <class Fn_>
        constexpr explicit impl_obj(Fn_&& fn) : fn(std::forward<Fn_>(fn)) {}
        impl_obj(impl_obj&&) = delete;
        Fn fn;
    };
    template <class T, class Fn, class =
        std::enable_if_t<is_range_v<T> && std::is_invocable_v<Fn, decltype(*begin(std::declval<T>()))>>>
    constexpr auto operator()(T&& rg, Fn&& fn) const {
        return subrange{filter_iterator{begin(rg), end(rg), fn}, filter_iterator{end(rg), end(rg), fn}};
    }
    template <class Fn>
    constexpr auto operator()(Fn&& fn) const { return impl_obj<Fn>{std::forward<Fn>(fn)}; }
} filter{};

} // namespace _filter

inline namespace _algo {

constexpr struct transform_t {
    template <class Fn>
    struct impl_obj : view_factory {
        template <class T, class = std::enable_if_t<is_range_v<T> && std::is_invocable_v<Fn, decltype(*begin(std::declval<T>()))>>>
        constexpr auto operator()(T&& rg) const {
            return subrange{transform_iterator{begin(rg), fn}, transform_iterator{end(rg), std::move(const_cast<impl_obj*>(this)->fn)}};
        }
        template <class Fn_>
        constexpr explicit impl_obj(Fn_&& fn) : fn(std::forward<Fn_>(fn)) {}
        impl_obj(impl_obj&&) = delete;
        Fn fn;
    };
    template <class T, class Fn, class =
        std::enable_if_t<is_range_v<T> && std::is_invocable_v<Fn, decltype(*begin(std::declval<T>()))>>>
    constexpr auto operator()(T&& rg, Fn&& fn) const {
        return subrange{transform_iterator{begin(rg), fn}, transform_iterator{end(rg), fn}};
    }
    template <class Fn>
    constexpr auto operator()(Fn&& fn) const { return impl_obj<Fn>{std::forward<Fn>(fn)}; }
} transform{};

} // namespace _algo

template <class T, class>
struct take_t::impl_func {
    constexpr auto operator()(T&& rg, size_t n) const {
        auto t = subrange{iota_iterator{size_t{0}}, iota_iterator{n}};
        return pack(t, rg) | transform([](auto&& i) {return std::get<1>(i);});
    }
};

} // namespace view

} // namespace vigna::range

namespace vigna {
namespace view = range::view;
}
