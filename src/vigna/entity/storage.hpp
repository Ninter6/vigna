//
// Created by Ninter6 on 2024/11/29.
//

#pragma once

#include <tuple>

#include "sparse_set.hpp"
#include "vigna/range/view.hpp"

namespace vigna {

template <class Entity, class T, class Alloc = std::allocator<T>, class = void>
class basic_storage : public basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>> {
    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, T>);

    using container_type = std::vector<T, Alloc>;

protected:
    using base_type = basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

    void swap_and_pop(size_t index) override {
        base_type::swap_and_pop(index);
        if (index != size() - 1)
            std::swap(payload_[index], payload_.back());
        payload_.pop_back();
    }

    using base_type::find_index;

public:
    using allocator_type = Alloc;
    using entity_type = Entity;
    using element_type = T;
    using value_type = element_type;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

    basic_storage() = default;

    [[nodiscard]] size_t size() const override { return payload_.size(); }
    [[nodiscard]] bool empty() const override { return payload_.empty(); }

    template<class... Args>
    std::pair<iterator, bool> emplace(Entity entity, Args&&... args) {
        assert(entity != null && base_type::size() == size());
        if (auto [it, success] = base_type::push(entity); success) {
            payload_.emplace_back(std::forward<Args>(args)...);
            return {end() - 1, true};
        } else {
            auto index = base_type::index(it);
            return {payload_.begin() + index, false};
        }
    }

    // template<class... Args>
    // std::pair<iterator, bool> replace(Entity entity, Args&&... args) {
    //     assert(entity != null && base_type::size() == size());
    //     if (auto it = find(entity); it != end()) {
    //         auto p = &*it; p->~T();
    //         new (p) T{std::forward<Args>(args)...};
    //         return {it, true};
    //     }
    //     return {end(), false};
    // }
    //
    // template<class... Args>
    // std::pair<iterator, bool> emplace_or_replace(Entity entity, Args&&... args) {
    //     assert(entity != null && base_type::size() == size());
    //     if (auto [it, success] = base_type::push(entity); success) {
    //         payload_.emplace_back(std::forward<Args>(args)...);
    //         return {end() - 1, true};
    //     } else {
    //         auto index = base_type::index(it);
    //         auto p = &payload_[index]; p->~T();
    //         new (p) T{std::forward<Args>(args)...};
    //         return {payload_.begin() + index, false};
    //     }
    // }

    std::pair<iterator, bool> push(Entity entity, const T& value) {
        return emplace(entity, value);
    }

    template <class First_, class Last_, class =
        std::enable_if_t<std::is_constructible_v<Entity, decltype(*std::declval<First_>())>, std::void_t<decltype(*++std::declval<First_>() != *std::declval<Last_>())>>>
    iterator insert(First_&& first, Last_&& last, const T& value) {
        for (auto it = first; it != last; ++it) emplace(*it, value);
        return end() - 1;
    }

    template <class First_, class Last_, class CFirst_, class = std::enable_if_t<
        std::is_constructible_v<Entity, decltype(*std::declval<First_>())> &&
        std::is_constructible_v<T,decltype(*std::declval<CFirst_>())>,
        std::void_t<decltype(*++std::declval<First_>() != *std::declval<Last_>(), *++std::declval<CFirst_>())>>>
    iterator insert(First_&& first, Last_&& last, CFirst_ values) {
        for (auto it = first; it != last; ++it, ++values) emplace(*it, *values);
        return end() - 1;
    }

    void erase(const iterator& it) {
        auto i = index(it);
        swap_and_pop(i);
    }

    void erase(iterator first, iterator last) {
        base_type::erase(first, last);
        while (first != last) erase(--last);
    }

    void erase(const Entity& entity) override {
        pop(entity);
    }

    void clear() override {
        base_type::clear();
        payload_.clear();
    }

    void pop(const Entity& entity) override {
        if (auto index = find_index(entity); index != null)
            swap_and_pop(index);
    }

    iterator find(const Entity& entity) {
        if (auto index = find_index(entity); index != null)
            return begin(index);
        return end();
    }
    // ReSharper disable once CppHidingFunction
    const_iterator find(const Entity& entity) const {
        if (auto index = find_index(entity); index != null)
            return cbegin(index);
        return cend();
    }

    T& get(const Entity& entity) {
        auto index = find_index(entity);
        assert(index != null && "Invalid entity!");
        return payload_[index];
    }
    const T& get(const Entity& entity) const {
        auto index = find_index(entity);
        assert(index != null && "Invalid entity!");
        return payload_[index];
    }

    T& operator[](const Entity& entity) {
        return *emplace(entity).first;
    }
    const T& operator[](const Entity& entity) const {
        return get(entity);
    }

    bool contains(const Entity& entity) const override {
        return find_index(entity) != null;
    }

    auto reach() { return payload_ | view::all; }
    auto reach() const { return payload_ | view::all; }

    auto each() { return view::pack(*this, payload_); }
    auto each() const { return view::pack(*this, payload_); }

    template<class...Fns, class = std::enable_if_t<(std::is_invocable_v<Fns, T>, ...)>>
    T& patch(const Entity& entity, Fns&&...f) {
        auto& e = get(entity);
        (std::forward<Fns>(f)(e), ...);
        return e;
    }
    template<class...Fns, class = std::enable_if_t<(std::is_invocable_v<Fns, std::add_const_t<T>>, ...)>>
    const T& patch(const Entity& entity, Fns&&...f) const {
        auto& e = get(entity);
        (std::forward<Fns>(f)(e), ...);
        return e;
    }

    // ReSharper disable CppHidingFunction
    T& front() { return payload_.front(); }
    T& back() { return payload_.back(); }
    const T& front() const { return payload_.front(); }
    const T& back() const { return payload_.back(); }

    iterator begin(size_t n = 0) { return payload_.begin() + n; }
    const_iterator begin(size_t n = 0) const { return payload_.begin() + n; }
    iterator end() { return payload_.end(); }
    const_iterator end() const { return payload_.end(); }
    const_iterator cbegin(size_t n = 0) const { return payload_.cbegin() + n; }
    const_iterator cend() const { return payload_.cend(); }
    reverse_iterator rbegin(size_t n = 0) { return payload_.rbegin() + n; }
    const_reverse_iterator rbegin(size_t n = 0) const { return payload_.rbegin() + n; }
    reverse_iterator rend() { return payload_.rend(); }
    const_reverse_iterator rend() const { return payload_.rend(); }
    const_reverse_iterator crbegin(size_t n = 0) const { return payload_.crbegin() + n; }
    const_reverse_iterator crend() const { return payload_.crend(); }
    // ReSharper restore CppHidingFunction

    using base_type::operator[];

    using base_type::index;
    size_t index(const const_iterator& it) const {
        return std::distance(cbegin(), it);
    }

private:
    container_type payload_;

};

template <class Entity, class Alloc>
class basic_storage<Entity, Entity, Alloc> : public basic_sparse_set<Entity, Alloc> {
    using traits = entity_traits<Entity>;
    static_assert(std::is_same_v<typename traits::entity_type, Entity>);

    using entity_value = typename traits::value_type;
    using id_type = typename traits::id_type;
    using version_type = typename traits::version_type;

    static constexpr id_type id(const Entity& entity) { return traits::id(entity); }
    static constexpr version_type version(const Entity& entity) { return traits::version(entity); }

protected:
    using base_type = basic_sparse_set<Entity, Alloc>;

    void swap_and_pop(size_t index) override {
        assert(index < length_);
        if (index != --length_)
            swap_elements_index(index, length_);
    } // swap only

    entity_value find_index(const Entity& value) const override {
        auto index = base_type::find_index(value);
        return valid((size_t)index) ? index : null;
    }

    using base_type::swap_elements_index;
    using base_type::reversion;

public:
    using allocator_type = Alloc;
    using entity_type = Entity;
    using element_type = Entity;
    using value_type = void;
    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using typename base_type::reverse_iterator;
    using typename base_type::const_reverse_iterator;

    using base_type::basic_sparse_set;

    basic_storage() = default;

    [[nodiscard]] size_t size() const override { return length_; }
    [[nodiscard]] bool empty() const override { return length_ == 0; }

    [[nodiscard]] size_t cemetery_size() const { return base_type::size() - length_; }
    [[nodiscard]] bool cemetery_empty() const { return base_type::size() == length_; }

    using base_type::capacity;
    using base_type::reserve;

    [[nodiscard]] bool valid(size_t index) const { return index < length_; }
    [[nodiscard]] bool valid(entity_type entity) const { return contains(entity); }

    entity_type emplace() {
        if (cemetery_empty()) base_type::emplace(length_, 0);
        return *begin(length_++);
    }

    // ReSharper disable once CppHidingFunction
    entity_type emplace(const entity_type& hint) {
        assert(hint != null && id(hint) < base_type::size());
        if (auto i = base_type::find_index(hint); !valid(i))
            swap_elements_index(length_++, i);
        return hint;
    }

    template <class First_, class Last_, class =
        std::enable_if_t<std::is_constructible_v<Entity, decltype(*std::declval<First_>())>, std::void_t<decltype(*++std::declval<First_>() != *std::declval<Last_>())>>>
    void insert(First_&& first, Last_&& last) {
        for (auto it = first; it != last; ++it) emplace(*it);
    }

    void erase(const iterator& it) override {
        swap_and_pop(index(it));
    }

    void erase(iterator first, iterator last) override {
        assert(last >= first);
        while (last != first) erase(--last);
    }

    void erase(const entity_type& value) override {
        pop(value);
    }

    void clear() override {
        base_type::clear();
        length_ = 0;
    }

    void pop(const entity_type& entity) override {
        if (auto index = find_index(entity); index != null)
            swap_and_pop(index);
    }

    // ReSharper disable once CppHidingFunction
    const_iterator find(const entity_type& entity) const {
        auto index = find_index(entity);
        return index != null ? begin(index) : end();
    }

    bool contains(const entity_type& value) const override {
        return find_index(value) != null;
    }

    // ReSharper disable CppHidingFunction
    const entity_type& front() { return *begin(); }
    const entity_type& back() { return *begin(length_ - 1); }

    using base_type::begin;
    using base_type::cbegin;
    using base_type::rend;
    using base_type::crend;
    [[nodiscard]] const_iterator end() const { return cbegin() + length_; }
    [[nodiscard]] const_iterator cend() const { return cbegin() + length_; }
    [[nodiscard]] const_reverse_iterator rbegin() const { return crend() - length_; }
    [[nodiscard]] const_reverse_iterator crbegin() const { return crend() - length_; }
    // ReSharper restore CppHidingFunction

    using base_type::operator[];

    using base_type::index;
    using base_type::swap_elements;

private:
    size_t length_{};

};

template <class Entity, class T, class Alloc>
class basic_storage<Entity, T, Alloc, VIGNA_ETO(T)> : public basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>> {
protected:
    using base_type = basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

public:
    using allocator_type = Alloc;
    using entity_type = Entity;
    using element_type = T;
    using value_type = void;
    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using typename base_type::reverse_iterator;
    using typename base_type::const_reverse_iterator;

    basic_storage() = default;

    using base_type::size;
    using base_type::empty;
    using base_type::capacity;
    using base_type::reserve;

    template<class...Args>
    std::pair<iterator, bool> emplace(const Entity& entity, Args&&...) {
        return base_type::emplace(entity);
    }

    template <class First_, class Last_, class...Args, class =
        std::enable_if_t<std::is_constructible_v<Entity, decltype(*std::declval<First_>())>, std::void_t<decltype(*++std::declval<First_>() != *std::declval<Last_>())>>>
    void insert(First_&& first, Last_&& last, Args&&...) {
        for (auto it = first; it != last; ++it) base_type::emplace(*it);
    }

    using base_type::erase;
    using base_type::clear;

    using base_type::find;
    using base_type::contains;

    void get(const Entity& entity) const {
        assert(contains(entity) && "Invalid entity!");
    }

    auto reach() const { return *this | view::all; }
    auto each() const { return reach(); }

    template<class...Fns, class = std::enable_if_t<(std::is_invocable_v<Fns> && ...)>>
    auto patch(const Entity& entity, Fns&&...f) const {
        assert(contains(entity) && "Invalid entity!");
        (std::forward<Fns>(f)(), ...);
    }

    using base_type::begin;
    using base_type::end;
    using base_type::cbegin;
    using base_type::cend;
    using base_type::rbegin;
    using base_type::rend;
    using base_type::crbegin;
    using base_type::crend;
    using base_type::operator[];

};

} // namespace vigna
