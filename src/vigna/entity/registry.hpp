//
// Created by Ninter6 on 2024/12/21.
//

#pragma once

#include "view.hpp"
#include "mixin.hpp"
#include "vigna/core/dense_map.hpp"
#include "vigna/reflect/utility.hpp"
#include "vigna/reflect/type_hash.hpp"

namespace vigna {

template <class, class>
class basic_registry;

namespace detail {

template <class T, class Alloc>
using mixin_type_t = VIGNA_MIXIN(basic_signal_mixin, T, basic_registry<typename T::entity_type, Alloc>);

template <class T, class Entity, class Alloc>
using storage_type_t = mixin_type_t<basic_storage<Entity, T, Alloc>,
    typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>>;

template <class T, class Entity, class Alloc>
using storage_for_t = reflect::constness_as_t<storage_type_t<std::remove_const_t<T>, Entity, Alloc>, T>;

}

template <class Entity = entity, class Alloc = std::allocator<Entity>>
class basic_registry {
    using alloc_traits = std::allocator_traits<Alloc>;
    static_assert(std::is_same_v<typename alloc_traits::value_type, Entity>, "Invalid value type");

    using base_type = basic_sparse_set<Entity, Alloc>;
    using traits = entity_traits<Entity>;

    using hash_value = entity_underlying_type;
    using pool_container_type = dense_map<hash_value, std::shared_ptr<base_type>,
        std::identity, std::equal_to<hash_value>,
        typename alloc_traits::template rebind_alloc<std::pair<const hash_value, std::shared_ptr<base_type>>>>;

    template <class T>
    using storage_for_type = detail::storage_for_t<T, Entity, typename alloc_traits::template rebind_alloc<T>>;

protected:
    template <class T>
    decltype(auto) assure(hash_value id = reflect::type_hash<T>()) {
        static_assert(std::is_same_v<T, std::decay_t<T>>, "Non-decayed types not allowed");
        if constexpr (std::is_same_v<T, Entity>) {
            assert(id == reflect::type_hash<Entity>() && "User entity storage not allowed");
            return entities_;
        } else {
            using storage_type = storage_for_type<T>;
            using alloc_type = typename alloc_traits::template rebind_alloc<storage_type>;

            if (auto it = pools_.find(id); it != pools_.end()) {
                assert(dynamic_cast<storage_type*>(&*it->second) != nullptr && "Unexpected storage type");
                return static_cast<storage_type&>(*it->second);
            }

            auto storage = std::allocate_shared<storage_type>(alloc_type{});
            pools_.emplace(id, storage);
            storage->bind(this);
            return *storage;
        }
    }

    template <class T>
    const auto* assure(hash_value id = reflect::type_hash<T>()) const {
        static_assert(std::is_same_v<T, std::decay_t<T>>, "Non-decayed types not allowed");
        if constexpr (std::is_same_v<T, Entity>) {
            assert(id == reflect::type_hash<Entity>() && "User entity storage not allowed");
            return &entities_;
        } else {
            using storage_type = storage_for_type<T>;

            auto it = pools_.find(id);
            if (it == pools_.end()) return nullptr;

            assert(dynamic_cast<storage_type*>(&*it->second) != nullptr && "Unexpected storage type");
            return static_cast<storage_type&>(&*it->second);
        }
    }

public:
    using allocator_type = Alloc;
    using entity_type = Entity;
    using version_type = typename traits::version_type;
    using common_type = base_type;

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

    template <class First_, class Last_>
    void create(First_&& first, Last_&& last) {
        entities_.insert(std::forward<First_>(first), std::forward<Last_>(last));
    }

    version_type destroy(const entity_type& entity) {
        for (auto&& [_, i] : pools_)
            i->pop(entity);
        entities_.erase(entity);
        return entities_.current(entity);
    }

    template <class First_, class Last_>
    void destroy(First_&& first, Last_&& last) {
        for (auto&& [_, i] : pools_)
            i->pop(std::forward<First_>(first), std::forward<Last_>(last));
        entities_.erase(std::forward<First_>(first), std::forward<Last_>(last));
    }

    template <class T, class...Args>
    decltype(auto) emplace(const entity_type& entity, Args&&...args) {
        assert(valid(entity) && "Invalid entity");
        return assure<T>().emplace(entity, std::forward<Args>(args)...);
    }

    template <class T, class First_, class Last_>
    void insert(First_&& first, Last_&& last, const T& value = {}) {
        assert(std::all_of(first, last, [&](auto&& e) { return valid(e); }) && "Invalid entity");
        assure<T>().insert(std::forward<First_>(first), std::forward<Last_>(last), value);
    }

    template <class T, class First_, class Last_, class CFirst_, class =
        std::enable_if_t<std::is_constructible_v<T, decltype(*std::declval<CFirst_>())>>>
    void insert(First_&& first, Last_&& last, CFirst_&& from) {
        assert(std::all_of(first, last, [&](auto&& e) { return valid(e); }) && "Invalid entity");
        assure<T>().insert(std::forward<First_>(first), std::forward<Last_>(last), std::forward<CFirst_>(from));
    }

    template <class T, class...Args, class = std::enable_if_t<
        !std::is_same_v<T, Entity> &&
        std::is_constructible_v<T, Args...>>>
    decltype(auto) emplace_or_replace(const entity_type& entity, Args&&...args) {
        if (auto& cpool = assure<T>(); cpool.contains(entity))
            return cpool.patch(entity, [&](auto&&...curr) { ((curr = T{std::forward<Args>(args)...}), ...); });
        else {
            cpool.emplace(entity, std::forward<Args>(args)...);
            return cpool.back();
        }
    }

    template <class T, class...Func>
    decltype(auto) patch(const entity_type& entity, Func&&...func) {
        return assure<T>().patch(entity, std::forward<Func>(func)...);
    }

    template <class T, class...Args>
    decltype(auto) replace(const entity_type& entity, Args&&...args) {
        return patch<T>(entity, [&](auto&...curr) { ((curr = T{std::forward<Args>(args)...}), ...); });
    }

    template <class T, class...Other>
    size_t remove(const entity_type& entity) {
        return (assure<T>().pop(entity) + ... + assure<Other>().pop(entity));
    }

    template <class T, class...Other, class First_, class Last_>
    size_t remove(First_ first, Last_ last) {
        size_t count{};

        if constexpr(std::is_same_v<First_, typename common_type::iterator>) {
            std::array cpools{static_cast<common_type *>(&assure<T>()), static_cast<common_type *>(&assure<Other>())...};
            for(auto from = cpools.begin(), to = cpools.end(); from != to; ++from) {
                if constexpr(sizeof...(Other) != 0u)
                    if((*from)->begin() <= first && first < (*from)->end())
                        std::swap(*from, cpools.back());
                count += (*from)->pop(first, last);
            }
        } else {
            for(auto cpools = std::forward_as_tuple(assure<T>(), assure<Other>()...); first != last; ++first)
                count += std::apply([e = *first](auto &...curr) { return (curr.pop(e) + ... + 0u); }, cpools);
        }

        return count;
    }

    template <class T, class...Other>
    void erase(const entity_type& entity) {
        (assure<T>().erase(entity), (assure<Other>().erase(entity), ...));
    }

    template <class T, class...Other, class First_, class Last_>
    size_t erase(First_ first, Last_ last) {
        size_t count{};

        if constexpr(std::is_same_v<First_, typename common_type::iterator>) {
            std::array cpools{static_cast<common_type *>(&assure<T>()), static_cast<common_type *>(&assure<Other>())...};
            for(auto from = cpools.begin(), to = cpools.end(); from != to; ++from) {
                if constexpr(sizeof...(Other) != 0u)
                    if((*from)->begin() <= first && first < (*from)->end())
                        std::swap(*from, cpools.back());
                count += (*from)->erase(first, last);
            }
        } else {
            for(auto cpools = std::forward_as_tuple(assure<T>(), assure<Other>()...); first != last; ++first)
                count += std::apply([e = *first](auto &...curr) { return (curr.erase(e) + ... + 0u); }, cpools);
        }

        return count;
    }

    template <class...Args>
    [[nodiscard]] bool all_of(const entity_type& entity) const {
        if constexpr (sizeof...(Args) == 0) {
            auto p = assure<Args...>();
            return p && p->contains(entity);
        } else {
            return (all_of<Args>(entity) && ...);
        }
    }

    template <class...Args>
    [[nodiscard]] bool any_of(const entity_type& entity) const {
        return (all_of<Args>(entity) || ...);
    }

    template<class...T>
    [[nodiscard]] decltype(auto) get(const entity_type& entity) const {
        if constexpr (sizeof...(T) == 1) {
            return (assure<T>()->get(entity), ...);
        } else {
            return std::forward_as_tuple(get<T>(entity)...);
        }
    }
    template<class...T>
    [[nodiscard]] decltype(auto) get(const entity_type& entity) {
        if constexpr (sizeof...(T) == 1) {
            return (assure<T>().get(entity), ...);
        } else {
            return std::forward_as_tuple(get<T>(entity)...);
        }
    }

    template <class T, class...Args>
    [[nodiscard]] decltype(auto) get_or_emplace(const entity_type& entity, Args&&...args) {
        auto& cpool = assure<T>();
        if (auto it = cpool.find(entity); it != cpool.end()) {
            return *it;
        } else {
            assert(valid(entity) && "Invalid entity");
            cpool.emplace(entity, std::forward<Args>(args)...);
            return cpool.back();
        }
    }

    template<class...Args>
    [[nodiscard]] auto try_get(const entity_type& entity) const {
        if constexpr (sizeof...(Args) == 1) {
            const auto *cpool = assure<Args...>();
            return (cpool && cpool->contains(entity)) ? std::addressof(cpool->get(entity)) : nullptr;
        } else {
            return std::make_tuple(try_get<Args>(entity)...);
        }
    }

    template<class...Args>
    [[nodiscard]] auto try_get(const entity_type& entity) {
        if constexpr (sizeof...(Args) == 1) {
            return (const_cast<Args*>(std::as_const(*this).template try_get<Args>(entity)), ...);
        } else {
            return std::make_tuple(try_get<Args>(entity)...);
        }
    }

    [[nodiscard]] size_t element_count(const entity& entity) const {
        assert(valid(entity) && "Invalid entity");
        return std::count_if(pools_.begin(), pools_.end(), [&](auto&&p) {
            p.second->contains(entity);
        });
    }

    [[nodiscard]] bool orphan(const entity_type& entity) const {
        assert(valid(entity) && "Invalid entity");
        return std::none_of(pools_.begin(), pools_.end(), [&](auto&&p) {
            p.second->contains(entity);
        });
    }

    template<class T>
    [[nodiscard]] auto on_construct(const hash_value id = reflect::type_hash<T>()) {
        return assure<T>(id).on_construct();
    }

    template<class T>
    [[nodiscard]] auto on_destroy(const hash_value id = reflect::type_hash<T>()) {
        return assure<T>(id).on_destroy();
    }

    template<class T>
    [[nodiscard]] auto on_update(const hash_value id = reflect::type_hash<T>()) {
        return assure<T>(id).on_update();
    }



private:
    pool_container_type pools_{};
    storage_for_type<Entity> entities_{this};

};

using registry = basic_registry<>;

}
