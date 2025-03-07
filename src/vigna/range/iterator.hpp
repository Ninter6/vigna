//
// Created by Ninter6 on 2024/12/1.
//

#pragma once

#include <limits>
#include <tuple>
#include <variant>

#include "range.hpp"

namespace vigna::range {

namespace detail {

struct default_filter {
    template <class T>
    constexpr bool operator()(T&&) const { return true; }
};

}

template <class T, class = std::enable_if_t<std::is_integral_v<T>>>
struct iota_iterator {
    using value_type = T;
    using pointer = void;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    iota_iterator() = default;
    constexpr explicit iota_iterator(T n) : value(n) {}

    T operator*() const {return value;}
    T operator[](T v) const {return value + v;}
    iota_iterator& operator++() {return ++value, *this;}
    iota_iterator operator++(int) {auto cp = *this; return ++value, cp;}
    iota_iterator& operator--() {return --value, *this;}
    iota_iterator operator--(int) {auto cp = *this; return --value, cp;}
    iota_iterator& operator+=(T n) {return value += n, *this;}
    iota_iterator& operator-=(T n) {return value -= n, *this;}
    iota_iterator operator+(T n) const {return iota_iterator(value + n);}
    iota_iterator operator-(T n) const {return iota_iterator(value - n);}
    std::ptrdiff_t operator-(const iota_iterator& o) const {return static_cast<std::ptrdiff_t>(value) - o.value;}
    bool operator==(const iota_iterator& other) const {return value == other.value;}
    bool operator!=(const iota_iterator& other) const {return value != other.value;}
    bool operator<(const iota_iterator& other) const {return value < other.value;}
    bool operator>(const iota_iterator& other) const {return value > other.value;}
    bool operator<=(const iota_iterator& other) const {return value <= other.value;}
    bool operator>=(const iota_iterator& other) const {return value >= other.value;}

    T value = std::numeric_limits<T>::max();
};

template <class T>
struct input_iterator_pointer {
    static_assert(std::is_same_v<T, std::decay<T>>, "Invalid type!");

    using value_type = T;
    using pointer = T*;
    using reference = T&;

    constexpr input_iterator_pointer() = default;
    constexpr explicit input_iterator_pointer(T&& inner) : inner(std::move(inner)) {}

    reference operator*() {
        return inner;
    }
    std::add_const_t<reference> operator*() const {
        return inner;
    }

    pointer operator->() {
        return std::addressof(inner);
    }
    std::add_const_t<pointer> operator->() const {
        return std::addressof(inner);
    }

    value_type inner;
};

template <class It, class...Args>
struct packed_iterator {
    using inner_type = decltype(std::make_tuple(std::declval<It>(), std::declval<Args>()...));
    using value_type = std::decay_t<decltype(std::forward_as_tuple(*std::declval<It>(), *std::declval<Args>()...))>;
    using pointer = input_iterator_pointer<value_type>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    constexpr packed_iterator() = default;
    constexpr explicit packed_iterator(const It& it, Args const& ... args)
        : it(it, args...) {}

    template <size_t...I>
    constexpr decltype(auto) _get(std::index_sequence<I...>) const {return std::forward_as_tuple(*std::get<I>(it)...);}
    template <size_t...I>
    constexpr void _increase(std::index_sequence<I...>) {(++std::get<I>(it), ...);}
    template <size_t...I>
    constexpr void _decrease(std::index_sequence<I...>) {(--std::get<I>(it), ...);}
    auto operator*() const {return _get(std::make_index_sequence<sizeof...(Args) + 1>{});}
    pointer operator->() const {return **this;}
    packed_iterator& operator++() {return _increase(std::make_index_sequence<sizeof...(Args) + 1>{}), *this;}
    packed_iterator operator++(int) {auto cp = *this; return _increase(std::make_index_sequence<sizeof...(Args) + 1>{}), cp;}
    packed_iterator& operator--() {return _decrease(std::make_index_sequence<sizeof...(Args) + 1>{}), *this;}
    packed_iterator operator--(int) {auto cp = *this; return _decrease(std::make_index_sequence<sizeof...(Args) + 1>{}), cp;}
    template <class...Args_>
    bool operator==(const packed_iterator<It, Args_...>& other) const {return std::get<0>(it) == std::get<0>(other.it);}
    template <class...Args_>
    bool operator!=(const packed_iterator<It, Args_...>& other) const {return std::get<0>(it) != std::get<0>(other.it);}

    inner_type it;
};

template <class It, class Fn = detail::default_filter>
struct filter_iterator {
    using value_type = std::remove_reference_t<decltype(*std::declval<It>())>;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    constexpr filter_iterator() = default;
    constexpr filter_iterator(const It& it_, const It& end_, const Fn& fn_)
        : it(it_), end(end_), fn(fn_) { if (it != end && !fn(*it)) ++*this; }
    constexpr filter_iterator(const It& it_, const It& end_, Fn&& fn_ = Fn{})
        : it(it_), end(end_), fn(std::move(fn_)) { if (it != end && !fn(*it)) ++*this; }

    decltype(auto) operator*() const {return *it;}
    pointer operator->() const {return std::addressof(*it);}
    filter_iterator& operator++() {
        assert(it != end);
        do ++it; while (it != end && !fn(*it));
        return *this;
    }
    filter_iterator operator++(int) {
        auto cp = *this; return ++it, cp;
    }
    template <class It_, class Fn_>
    bool operator==(const filter_iterator<It_, Fn_>& other) const {return it == other.it;}
    template <class It_, class Fn_>
    bool operator!=(const filter_iterator<It_, Fn_>& other) const {return it != other.it;}

    It it, end;
    Fn fn;
};

template <class It, class Fn = identity>
struct transform_iterator {
    using inner_it_value_t = decltype(*std::declval<It>());
    using value_type = std::invoke_result_t<Fn, decltype(*std::declval<It>())>;
    using pointer = input_iterator_pointer<std::decay_t<value_type>>;
    using reference = std::conditional_t<
            std::is_rvalue_reference_v<inner_it_value_t> ||
            std::is_same_v<inner_it_value_t, std::decay_t<inner_it_value_t>>,
        std::decay_t<value_type>, value_type>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    constexpr transform_iterator() = default;
    constexpr explicit transform_iterator(const It& it, const Fn& fn)
        : it(it), fn(fn) {}
    constexpr explicit transform_iterator(const It& it, Fn&& fn = Fn{})
        : it(it), fn(std::move(fn)) {}

    reference operator*() const {return fn(*it);}
    pointer operator->() const {return fn(*it);}
    transform_iterator& operator++() {return ++it, *this;}
    transform_iterator operator++(int) {auto cp = *this; return ++it, cp;}
    transform_iterator& operator--() {return --it, *this;}
    transform_iterator operator--(int) {auto cp = *this; return --it, cp;}
    template <class It_, class Fn_>
    bool operator==(const transform_iterator<It_, Fn_>& other) const {return it == other.it;}
    template <class It_, class Fn_>
    bool operator!=(const transform_iterator<It_, Fn_>& other) const {return it != other.it;}

    It it;
    Fn fn;
};

template <class It, class Sen, class = std::enable_if_t<is_compatible_iterator_v<It, Sen>>>
struct common_iterator {
    using inner_type = std::variant<It, Sen>;
    using result_type = decltype(*std::declval<It>());
    using value_type = std::remove_reference_t<result_type>;
    using pointer = value_type*;
    using reference = result_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    common_iterator() = default;

    template<class T, class =
        std::enable_if_t<std::is_same_v<std::decay_t<T>, It> || std::is_same_v<std::decay_t<T>, Sen>>>
    explicit common_iterator(T&& it) : inner(std::forward<T>(it)) {}

    decltype(auto) operator*() const {
        return std::visit([](auto&& it) -> result_type {
            if constexpr (std::is_same_v<result_type, decltype(*it)>)
                return *it;
            else assert(false && "Invalid dereferencing");
        }, inner);
    }
    pointer operator->() const { return std::addressof(deref_(inner)); }
    common_iterator& operator++() { return std::visit([](auto&&i){++i;}, inner), *this; }
    common_iterator operator++(int) { auto cp = *this; return inc_(inner), cp; }
    bool operator==(const common_iterator& o) const { return std::visit([](auto&& a, auto&& b) { return a == b; }, inner, o.inner); }
    bool operator!=(const common_iterator& o) const { return !(*this == o); }

    inner_type inner;
};

}
