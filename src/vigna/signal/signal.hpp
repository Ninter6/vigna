//
// Created by Ninter6 on 2024/12/10.
//

#pragma once

#include "delegate.hpp"
#include "vigna/core/dense_map.hpp"

namespace vigna {

template <class Delegate, class Alloc = std::allocator<std::pair<const connection, Delegate>>>
class signal;

template <template<class...> class Delegate, class...Args, class Alloc>
class signal<Delegate<Args...>, Alloc> {
    using call_t = Delegate<Args...>;
    using alloc_traits = std::allocator_traits<Alloc>;
    using conn_alloc = typename alloc_traits::template rebind_alloc<bool>;
    using container = compressed_pair<
        dense_map<connection, call_t,
            std::hash<connection>,
            std::equal_to<connection>,
            typename alloc_traits::template rebind_alloc<std::pair<const connection, call_t>>>,
        conn_alloc>;

public:
    signal() = default;

    template<class Fn, class = std::enable_if_t<std::is_constructible_v<call_t, Fn>>>
    connection connect(Fn&& fn) {
        call_t call{calls_.second(), fn};
        auto conn = call.get_connection();
        calls_.first().emplace(conn, std::move(call));
        return conn;
    }

    template<auto Fn, class...Args_, class = std::enable_if_t<
        std::is_member_function_pointer_v<decltype(Fn)> ||
        std::is_invocable_v<decltype(Fn), Args_..., Args...>>>
    connection connect(Args_&&...obj_or_none) {
        call_t call{calls_.second()};
        auto conn = call.template connect<Fn>(std::forward<Args_>(obj_or_none)...);
        calls_.first().emplace(conn, std::move(call));
        return conn;
    }

    void disconnect(const connection& conn) {
        conn.release();
        calls_.first().erase(conn);
    }

    void emit(Args&&...args) {
        for (auto it = calls_.first().begin(); it != calls_.first().end();)
            switch (it->second(std::forward<Args>(args)...)) {
                default:
                case signal_r::keep: ++it; break;
                case signal_r::erase: calls_.first().erase(it); break;
            }
    }

private:
     container calls_;

};

template <class...Args>
using signal_for = signal<delegate<Args...>>;

template <class Alloc, class...Args>
using signal_alloc = signal<delegate<Args...>, Alloc>;

}
