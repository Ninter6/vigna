//
// Created by Ninter6 on 2024/12/26.
//

#pragma once

#include "signal.hpp"

namespace vigna {

template <class>
class sink;

template <template<class...> class Delegate, class...Args, class Alloc>
class sink<signal<Delegate<Args...>, Alloc>> {
    using call_t = Delegate<Args...>;
    using signal_type = signal<call_t, Alloc>;

public:
    sink() = default;
    explicit sink(signal_type& signal) : signal_(&signal) {}

    void bind(signal_type& signal) { signal_ = &signal; }

    [[nodiscard]] size_t size() const { return assert(signal_), signal_->size(); }
    [[nodiscard]] bool empty() const { return assert(signal_), signal_->empty(); }

    template<class Fn, class = std::enable_if_t<std::is_constructible_v<call_t, Fn>>>
    connection connect(Fn&& fn) {
        return assert(signal_), signal_->connect(std::forward<Fn>(fn));
    }

    template<auto Fn, class...Args_, class = std::enable_if_t<
        std::is_member_function_pointer_v<decltype(Fn)> ||
        std::is_invocable_v<decltype(Fn), Args_..., Args...>>>
    connection connect(Args_&&...obj_or_none) {
        return assert(signal_), signal_->template connect<Fn>(std::forward<Args_>(obj_or_none)...);
    }

    void disconnect(const connection& conn) { assert(signal_), signal_->disconnect(conn); }
    void clear() { assert(signal_), signal_->clear(); }

private:
    signal_type* signal_{};

};

}
