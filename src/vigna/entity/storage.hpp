//
// Created by Ninter6 on 2024/11/29.
//

#pragma once

#include <tuple>

#include "sparse_set.hpp"
#include "vigna/range/iterator.hpp"

namespace vigna {

namespace detail {

// template <class It, class... Other>
// struct extended_storage_iterator {
//     using iterator_type = It;
//     using value_type = decltype(std::tuple_cat(std::make_tuple(*std::declval<It>()), std::forward_as_tuple(*std::declval<Other>()...)));
//     using pointer = range::input_iterator_pointer<value_type>;
//     using reference = value_type;
//     using difference_type = std::ptrdiff_t;
//     using iterator_category = std::input_iterator_tag;
//     using iterator_concept = std::forward_iterator_tag;
//
//
// private:
//     std::tuple<It, Other...> it_;
//
// };

}

template <class Entity, class T, class Alloc = std::allocator<T>, class = void>
class basic_storage : public basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>> {
    using base_type = basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, T>);

    using container_type = std::vector<T, Alloc>;

protected:
    void swap_and_pop(size_t index) override {
        base_type::swap_and_pop(index);
        if (index != size() - 1)
            std::swap(payload_[index], payload_.back());
        payload_.pop_back();
    }

    using base_type::find_index;

public:
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

    using base_type::basic_sparse_set;

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

    template<class... Args>
    std::pair<iterator, bool> replace(Entity entity, Args&&... args) {
        assert(entity != null && base_type::size() == size());
        if (auto it = find(entity); it != end()) {
            auto p = &*it; p->~T();
            new (p) T{std::forward<Args>(args)...};
            return {it, true};
        }
        return {end(), false};
    }

    template<class... Args>
    std::pair<iterator, bool> emplace_or_replace(Entity entity, Args&&... args) {
        assert(entity != null && base_type::size() == size());
        if (auto [it, success] = base_type::push(entity); success) {
            payload_.emplace_back(std::forward<Args>(args)...);
            return {end() - 1, true};
        } else {
            auto index = base_type::index(it);
            auto p = &payload_[index]; p->~T();
            new (p) T{std::forward<Args>(args)...};
            return {payload_.begin() + index, false};
        }
    }

    std::pair<iterator, bool> push(Entity entity, const T& value) {
        return emplace(entity, value);
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

    T& at(const Entity& entity) {
        auto index = find_index(entity);
        assert(index != null);
        return payload_[index];
    }
    const T& at(const Entity& entity) const {
        auto index = find_index(entity);
        assert(index != null);
        return payload_[index];
    }

    T& operator[](const Entity& entity) {
        return *emplace(entity).first;
    }
    const T& operator[](const Entity& entity) const {
        return at(entity);
    }

    bool contains(const Entity& entity) const override {
        return find_index(entity) != null;
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

    using base_type::index;
    size_t index(const const_iterator& it) const {
        return std::distance(cbegin(), it);
    }

private:
    container_type payload_;

};

template <class Entity, class Alloc>
class basic_storage<Entity, Entity, Alloc> : public basic_sparse_set<Entity, Alloc> {
    using base_type = basic_sparse_set<Entity, Alloc>;

    using traits = typename base_type::traits;
    static_assert(std::is_same_v<typename traits::entity_type, Entity>);

    using entity_type = typename traits::entity_type;
    using value_type = typename traits::value_type;
    using id_type = typename traits::id_type;
    using version_type = typename traits::version_type;

protected:

public:
    using base_type::iterator;
    using base_type::const_iterator;
    using base_type::reverse_iterator;
    using base_type::const_reverse_iterator;

    using base_type::basic_sparse_set;

    basic_storage() = default;

    entity_type create() {

    }

private:
    size_t length_{};

};

} // namespace vigna
