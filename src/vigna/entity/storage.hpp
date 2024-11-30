//
// Created by Ninter6 on 2024/11/29.
//

#pragma once

#include "sparse_set.hpp"

namespace vigna {

template <class Entity, class T, class Alloc = std::allocator<T>, class = void>
class basic_storage : public basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>> {
    using base_type = basic_sparse_set<Entity, typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, Entity>);

    using container_type = std::vector<T, Alloc>;

public:
    using base_type::basic_sparse_set;

    basic_storage() = default;

private:
    container_type payload;

};

} // namespace vigna
