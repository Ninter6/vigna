//
// Created by Ninter6 on 2024/11/29.
//

#pragma once

#include <tuple>

#include "sparse_set.hpp"
#include "vigna/range/iterator.hpp"

namespace vigna {

namespace detail {

template <class It, class... Other>
struct extended_storage_iterator {
    using iterator_type = It;
    using value_type = decltype(std::tuple_cat(std::make_tuple(*std::declval<It>()), std::forward_as_tuple(*std::declval<Other>()...)));
    using pointer = input_iterator_pointer<value_type>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;


private:
    std::tuple<It, Other...> it_;

};

}

template <class Entity, class T, class Alloc = std::allocator<T>, class = void>
class basic_storage : public basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>> {
    using base_type = basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, T>);

    using container_type = std::vector<T, Alloc>;

public:
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    using base_type::basic_sparse_set;

    basic_storage() = default;

    [[nodiscard]] size_t size() const override { return payload.size(); }
    [[nodiscard]] bool empty() const override { return payload.empty(); }

    template<class... Args>
    iterator emplace(Entity entity, Args&&... args) {
        assert(entity != null && base_type::size() == size());
//        if (auto it = find(entity); it != end())
//            return replace(it, args...);
        base_type::push(entity);
        return payload.emplace(std::forward<Args>(args)...);
    }

    iterator push(Entity entity, const T& value) {
        assert(entity != null && base_type::size() == size());
//        if (auto it = find(entity); it != end())
//            return replace(it, value);
        base_type::push(entity);
        return payload.push_back(value);
    }

private:
    container_type payload;

};

} // namespace vigna
