//
// Created by Ninter6 on 2024/12/21.
//

#pragma once

#include "mixin.hpp"
#include "vigna/core/dense_map.hpp"
#include "vigna/reflect/utility.hpp"

namespace vigna {

template <class, class>
class basic_registry;

namespace detail {

template <class T, class Alloc>
using mixin_type_t = basic_signal_mixin<T, basic_registry<typename T::entity_type, Alloc>>;

template <class T, class Entity, class Alloc>
using storage_type_t = mixin_type_t<basic_storage<Entity, T, Alloc>,
    typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

template <class T, class Entity, class Alloc>
using storage_for_t = reflect::constness_as_t<storage_type_t<std::remove_const_t<T>, Entity, Alloc>, T>;

}

template <class Entity = entity, class Alloc = std::allocator<Entity>>
struct basic_registry {
    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, Entity>, "Invalid value type");

    using base_type = basic_sparse_set<Entity, Alloc>;
    using traits = entity_traits<Entity>;

    using hash_value = entity_underlying_type;
    using pool_container_type = dense_map<hash_value, std::shared_ptr<base_type>,
        std::identity, std::equal_to<hash_value>,
        typename alloc_traits::template rebind_alloc<std::pair<const hash_value, std::shared_ptr<base_type>>>>;

    template<class T>
    using storage_for_type = detail::storage_for_t<T, Entity, typename alloc_traits::template rebind_alloc<T>>;

public:
    using allocator_type = Alloc;
    using entity_type = Entity;
    using version_type = traits::version_type;

    basic_registry() = default;

    [[nodiscard]] bool valid(const entity_type& entity) const {
        return entities_.valid(entity);
    }

    [[nodiscard]] version_type current(const entity_type& entity) const {
        return entities_.current(entity);
    }

    auto create() {
        return entities_.emplace();
    }

    auto create(const entity_type& hint) {
        return entities_.emplace(hint);
    }

    template<class First_, class Last_>
    void create(First_&& first, Last_&& last) {
        entities_.insert(std::forward<First_>(first), std::forward<Last_>(last));
    }

    version_type destroy(const entity_type entity) {
        for (auto&& [_, i] : pool_)
            i->pop(entity);
        entities_.erase(entity);
        return entities_.current(entity);
    }

    template<class First_, class Last_>
    void destroy(First_&& first, Last_&& last) {
        for (auto&& [_, i] : pool_)
            i->pop(std::forward<First_>(first), std::forward<Last_>(last));
        entities_.erase(std::forward<First_>(first), std::forward<Last_>(last));
    }

private:
    pool_container_type pool_{};
    storage_for_type<Entity> entities_{this};

};

using registry = basic_registry<>;

}
