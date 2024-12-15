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
    using container = dense_map<connection, call_t,
        std::hash<connection>,
        std::equal_to<connection>,
        typename alloc_traits::template rebind_alloc<std::pair<const connection, call_t>>>;

public:
    signal() = default;

    template<class Fn, class = std::enable_if_t<std::is_constructible_v<call_t, Fn>>>
    connection connect(Fn&& fn) {
        call_t call{fn};
        auto conn = call.get_connection();
        calls_.emplace(conn, std::move(call));
        return conn;
    }

    template<auto Fn, class...Args_, class = std::enable_if_t<
        std::is_invocable_v<decltype(Fn), Args_..., Args...>>>
    connection connect(Args_&&...obj_or_none) {
        call_t call{};
        auto conn = call.template connect<Fn>(std::forward<Args_>(obj_or_none)...);
        calls_.emplace(conn, std::move(call));
        return conn;
    }

    void disconnect(const connection& conn) {
        conn.release();
        calls_.erase(conn);
    }

    void emit(Args&&...args) {
        for (auto it = calls_.begin(); it != calls_.end();)
            switch (it->second(std::forward<Args>(args)...)) {
                case signal_r::keep: ++it; break;
                case signal_r::erase: calls_.erase(it); break;
            }
    }

private:
     container calls_;

};

}
