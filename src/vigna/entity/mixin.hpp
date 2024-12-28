//
// Created by Ninter6 on 2024/12/21.
//

#pragma once

#include "vigna/signal/signal.hpp"
#include "vigna/signal/sink.hpp"

namespace vigna {

namespace detail {

template<class, class, class = void>
struct has_on_construct final: std::false_type {};

template<class Type, class Registry>
struct has_on_construct<Type, Registry, std::void_t<decltype(Type::on_construct(std::declval<Registry&>(), std::declval<typename Registry::entity_type>()))>>
    : std::true_type {};

template<class, class, class = void>
struct has_on_destroy final: std::false_type {};

template<class Type, class Registry>
struct has_on_destroy<Type, Registry, std::void_t<decltype(Type::on_destroy(std::declval<Registry&>(), std::declval<typename Registry::entity_type>()))>>
    : std::true_type {};

template<class, class, class = void>
struct has_on_update final: std::false_type {};

template<class Type, class Registry>
struct has_on_update<Type, Registry, std::void_t<decltype(Type::on_update(std::declval<Registry&>(), std::declval<typename Registry::entity_type>()))>>
    : std::true_type {};

}

template <class Type, class Registry>
class basic_signal_mixin final: public Type {
    using underlying_type = Type;
    using owner_type = Registry;

    using signal_type = signal_alloc<typename underlying_type::allocator_type, owner_type&, const typename underlying_type::entity_type>;
    using sink_type = sink<signal_type>;

    void auto_connect() {
        if constexpr(detail::has_on_construct<typename underlying_type::element_type, Registry>::value) {
            construction_.template connect<&underlying_type::element_type::on_construct>();
        }
        if constexpr(detail::has_on_destroy<typename underlying_type::element_type, Registry>::value) {
            destruction_.template connect<&underlying_type::element_type::on_destroy>();
        }
        if constexpr(detail::has_on_update<typename underlying_type::element_type, Registry>::value) {
            update_.template connect<&underlying_type::element_type::on_update>();
        }
    }

    owner_type& owner_or_assert() const {
        return assert(owner_), *owner_;
    }

    void swap_and_pop(size_t index) final {
        assert(index < underlying_type::size());
        destruction_.emit(owner_or_assert(), underlying_type::operator[](index));
        underlying_type::swap_and_pop(index);
    }

public:
    using allocator_type = typename underlying_type::allocator_type;
    using entity_type = typename underlying_type::entity_type;
    using registry_type = owner_type;

    basic_signal_mixin() { auto_connect(); }
    explicit basic_signal_mixin(owner_type* owner) : owner_(owner) { auto_connect(); }

    void bind(registry_type& owner) { owner_ = &owner; }

    auto on_construct() { return sink_type{construction_}; }
    auto on_destroy() { return sink_type{destruction_}; }
    auto on_update() { return sink_type{update_}; }

    auto emplace() {
        const auto entity = underlying_type::emplace();
        construction_.emit(owner_or_assert(), entity);
        return entity;
    }

    template <class...Args>
    decltype(auto) emplace(entity_type hint, Args&&...args) {
        if constexpr (std::is_same_v<entity_type, typename underlying_type::element_type>) {
            if (!underlying_type::contains(hint)) {
                auto elem = underlying_type::emplace(hint, std::forward<Args>(args)...);
                construction_.emit(owner_or_assert(), elem);
            }
            return hint;
        } else {
            auto [it, succ] = underlying_type::emplace(hint, std::forward<Args>(args)...);
            if (succ) construction_.emit(owner_or_assert(), hint);
            return *it;
        }
    }

    template <class First_, class Last_, class...Args>
    auto insert(First_&& first, Last_&& last, Args&&...args) {
        auto from = underlying_type::size();
        underlying_type::insert(first, last, std::forward<Args>(args)...);
        if(auto &reg = owner_or_assert(); !construction_.empty())
            for(const auto to = underlying_type::size(); from != to; ++from)
                construction_.emit(reg, underlying_type::operator[](from));
    }

    void clear() final {
        if (!destruction_.empty())
            for (size_t i = 0; i < underlying_type::size(); ++i)
                destruction_.emit(owner_or_assert(), underlying_type::operator[](i));
        underlying_type::clear();
    }

    template<class...Func>
    decltype(auto) patch(const entity_type entt, Func&&...func) {
        underlying_type::patch(entt, std::forward<Func>(func)...);
        update_.emit(owner_or_assert(), entt);
        return this->get(entt);
    }

private:
    owner_type* owner_{};
    signal_type construction_{};
    signal_type destruction_{};
    signal_type update_{};

};

}
