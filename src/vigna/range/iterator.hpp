//
// Created by Ninter6 on 2024/12/1.
//

#pragma once

#include <limits>
#include <tuple>

namespace vigna::range {

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
    constexpr explicit input_iterator_pointer(T&& inner) : inner(std::forward<T>(inner)) {}

    reference operator*() {
        return inner;
    }
    std::add_const_t<reference> operator*() const {
        return inner;
    }

    pointer operator->() {
        return &inner;
    }
    std::add_const_t<pointer> operator->() const {
        return &inner;
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
    constexpr explicit packed_iterator(It&& it, Args&&... args)
    : it(std::make_tuple(std::forward<It>(it), std::forward<Args>(args)...)) {}

    template <size_t...I>
    constexpr decltype(auto) _get(std::index_sequence<I...>) const {return std::forward_as_tuple(*std::get<I>(it)...);}
    template <size_t...I>
    constexpr void _increase(std::index_sequence<I...>) {(++std::get<I>(it), ...);}
    auto operator*() const {return _get(std::make_index_sequence<sizeof...(Args) + 1>{});}
    pointer operator->() const {return **this;}
    packed_iterator& operator++() {return _increase(std::make_index_sequence<sizeof...(Args) + 1>{}), *this;}
    packed_iterator operator++(int) {auto cp = *this; return _increase(std::make_index_sequence<sizeof...(Args) + 1>{}), cp;}
    template <class...Args_>
    bool operator==(const packed_iterator<It, Args_...>& other) const {return std::get<0>(it) == std::get<0>(other.it);}
    template <class...Args_>
    bool operator!=(const packed_iterator<It, Args_...>& other) const {return std::get<0>(it) != std::get<0>(other.it);}

    inner_type it;
};

template <class It, class Fn = std::identity>
struct transform_iterator {
    using value_type = std::invoke_result_t<Fn, decltype(*std::declval<It>())>;
    using pointer = input_iterator_pointer<value_type>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    constexpr transform_iterator() = default;
    constexpr explicit transform_iterator(It&& it, Fn&& fn = Fn{}) : it(std::forward<It>(it)), fn(std::forward<Fn>(fn)) {}

    decltype(auto) operator*() const {return fn(*it);}
    pointer operator->() const {return fn(*it);}
    transform_iterator& operator++() {return ++it, *this;}
    transform_iterator operator++(int) {auto cp = *this; return ++it, cp;}
    transform_iterator& operator--() {return --it, *this;}
    transform_iterator operator--(int) {auto cp = *this; return --it, cp;}
    template <class Fn_>
    bool operator==(const transform_iterator<It, Fn_>& other) const {return it == other.it;}
    template <class Fn_>
    bool operator!=(const transform_iterator<It, Fn_>& other) const {return it != other.it;}

    std::decay_t<It> it;
    std::decay_t<Fn> fn;
};

}
