//
// Created by Ninter6 on 2024/11/25.
//

#pragma once

#include "entity.hpp"

#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <functional>

namespace vigna {

template <class T, class Alloc = std::allocator<T>>
class basic_sparse_set {
    using traits = entity_traits<T>;
    static_assert(std::is_same_v<typename traits::entity_type, T>);
    static_assert(sizeof(typename traits::value_type) == sizeof(T));

    using value_type = typename traits::value_type;
    using id_type = typename traits::id_type;
    using version_type = typename traits::version_type;

    static constexpr size_t sparse_page_size = VIGNA_SPARSE_PAGE;
    struct sparse_page_deleter : private Alloc { void operator()(value_type* ptr) {
        Alloc::deallocate(reinterpret_cast<typename alloc_traits::pointer>(ptr), sparse_page_size);
    }};
    using sparse_page = std::unique_ptr<value_type[], sparse_page_deleter>;

    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, T>);
    using sparse_container = std::vector<sparse_page, typename alloc_traits::template rebind_alloc<sparse_page>>;
    using packed_container = std::vector<T, Alloc>;

    static constexpr id_type id(const T& entity) { return traits::id(entity); }
    static constexpr version_type version(const T& entity) { return traits::version(entity); }

    static constexpr std::pair<size_t, size_t> sparse_bise(id_type id) {
        return {id / sparse_page_size, id % sparse_page_size};
    }

    void sparse_emplace(id_type id, value_type index) {
        auto [i, j] = sparse_bise(id);
        if (i >= sparse_.size()) sparse_.resize(i + 1);
        if (sparse_[i] == nullptr) {
            sparse_[i].reset(reinterpret_cast<value_type*>(packed_.get_allocator().allocate(sparse_page_size)));
            std::uninitialized_fill_n(sparse_[i].get(), sparse_page_size, null); // as we have the useful 'null', instead of 'null_index'
        }
        sparse_[i][j] = index;
    }

    value_type& sparse_at(id_type id) {
        auto [i, j] = sparse_bise(id);
        assert(i < sparse_.size() && sparse_[i]);
        return sparse_[i][j];
    }
    const value_type& sparse_at(id_type id) const {
        auto [i, j] = sparse_bise(id);
        assert(i < sparse_.size() && sparse_[i]);
        return sparse_[i][j];
    }

    void isolate(id_type id) { // sparse erase
        sparse_at(id) = null;
    }

protected:
    void swap_and_pop(size_t index) {
        assert(index < packed_.size());
        isolate(id(packed_[index]));
        if (index != packed_.size() - 1) {
            sparse_at(id(packed_.back())) = static_cast<value_type>(index);
            std::swap(packed_[index], packed_.back());
        }
        packed_.pop_back();
    }

    value_type find_index(const T& value) const {
        auto [i, j] = sparse_bise(id(value));
        if (i < sparse_.size() && sparse_[i]) return sparse_[i][j];
        return null;
    }

    void swap_elements_index(size_t a, size_t b) {
        assert(a < packed_.size() && b < packed_.size());
        std::swap(sparse_at(packed_[a]), sparse_at(packed_[b]));
        std::swap(packed_[a], packed_[b]);
    }

public:
    using iterator = typename packed_container::const_iterator;
    using const_iterator = iterator;

    basic_sparse_set() = default;
    basic_sparse_set(const basic_sparse_set&) = delete;
    basic_sparse_set(basic_sparse_set&&) = default;

    virtual ~basic_sparse_set() = default;

    [[nodiscard]] virtual size_t size() const { return packed_.size(); }
    [[nodiscard]] virtual bool empty() const { return packed_.empty(); }

    [[nodiscard]] size_t capacity() const { return packed_.capacity(); }
    void reserve(size_t size) { packed_.reserve(size); }
    void shrink_to_fit() { packed_.shrink_to_fit(); }

    iterator emplace(id_type id, version_type version = 0) {
        return push(traits::construct(id, version));
    }
    iterator emplace() {
        return push(traits::construct(size(), 0));
    }

    iterator push(const T& value) {
        if (contains(value)) return end();
        auto index = packed_.size();
        packed_.push_back(value);
        sparse_emplace(id(value), index);
        return begin(index);
    }

    void erase(const iterator& it) {
        auto index = std::distance(packed_.cbegin(), it);
        swap_and_pop(index);
    }

    void erase(const iterator& first, const iterator& last) {
        assert(last >= first);
        while (last != first) erase(--last);
    }

    void erase(const T& value) {
        pop(value);
    }

    void clear() {
        for (auto&& i : packed_)
            isolate(id(i));
        packed_.clear();
    }

    void pop(const T& value) {
        erase(find(value));
    }

    const_iterator find(const T& value) const {
        auto index = find_index(value);
        return index != null ? begin(index) : end();
    }

    bool contains(const T& value) const {
        return find_index(value) != null;
    }

    const T& front() { return packed_.front(); }
    const T& back() { return packed_.back(); }

    const_iterator begin(size_t n = 0) const { return packed_.begin() + n; }
    const_iterator end() const { return packed_.end(); }
    const_iterator cbegin(size_t n = 0) const { return packed_.cbegin() + n; }
    const_iterator cend() const { return packed_.cend(); }
    auto rbegin(size_t n = 0) const { return packed_.rbegin() + n; }
    auto rend() const { return packed_.rend(); }
    auto crbegin(size_t n = 0) const { return packed_.crbegin() + n; }
    auto crend() const { return packed_.crend(); }

    void swap_elements(const iterator& a, const iterator& b) {
        auto ai = std::distance(packed_.cbegin(), a);
        auto bi = std::distance(packed_.cbegin(), b);
        swap_elements_index(ai, bi);
    }

    void sort(const std::function<bool(T, T)>& camp = [](const T& a, const T& b) {
        return id(a) < id(b);
    }) {
        std::sort(packed_.begin(), packed_.end(), camp);
        for (value_type i = 0; i < packed_.size(); ++i)
            sparse_at(id(packed_[i])) = i;
    }

    void partition(const std::function<bool(T)>& pre) {
        std::partition(packed_.begin(), packed_.end(), pre);
        for (value_type i = 0; i < packed_.size(); ++i)
            sparse_at(id(packed_[i])) = i;
    }


    bool is_sorted(const std::function<bool(T, T)>& camp = [](const T& a, const T& b) {
        return id(a) < id(b);
    }) const {
        return std::is_sorted(begin(), end(), camp);
    }

    void is_partitioned(const std::function<bool(T)>& pre) const {
        return std::is_partitioned(begin(), end(), pre);
    }

private:
    sparse_container sparse_;
    packed_container packed_;

};

} // namespace vigna
