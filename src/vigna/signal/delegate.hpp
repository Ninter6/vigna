//
// Created by Ninter6 on 2024/12/11.
//

#pragma once

#include <memory>
#include <functional>

#include "vigna/reflect/function_traits.hpp"

namespace vigna {

enum class signal_r {
    keep, erase
};

struct connection {
    connection() = default;
    explicit connection(const std::weak_ptr<bool>& connected)
        : connected(connected) {}

    [[nodiscard]] explicit operator bool() const {
        auto p = connected.lock();
        return p != nullptr ? *p : false;
    }

    void release() const {
        auto p = connected.lock();
        if (p != nullptr) *p = false;
    }

private:
    std::weak_ptr<bool> connected;

};

template <class...Args>
class delegate {
    using Fn = signal_r(Args...);

    template <class, class>
    friend class signal;

public:
    delegate() = default;
    template <class Fn_, class = std::enable_if_t<std::is_invocable_v<Fn_, Args...>>>
    explicit delegate(Fn_&& fn) { connect(std::forward<Fn_>(fn)); }

    delegate(const delegate&) = delete;
    delegate(delegate&&) = default;
    delegate& operator=(const delegate&) = delete;
    delegate& operator=(delegate&&) = default;

    signal_r operator()(Args&&...args) const {
        if (!*connected_) return signal_r::erase;
        return call_(std::forward<Args>(args)...);
    }

    template <class Fn_, class = std::enable_if_t<std::is_invocable_v<Fn_, Args...>>>
    connection connect(Fn_&& fn) {
        if constexpr (std::is_invocable_r_v<bool, Fn_, Args...>)
            call_ = [fn = std::forward<Fn_>(fn)](Args&&...args) mutable {
                return fn(std::forward<Args>(args)...) ? signal_r::keep : signal_r::erase;
            }; // true: keep, false: erase
        else if constexpr (std::is_invocable_r_v<void, Fn_, Args...>)
            call_ = [fn = std::forward<Fn_>(fn)](Args&&...args) mutable {
                return fn(std::forward<Args>(args)...), signal_r::keep;
            };
        else call_ = std::forward<Fn_>(fn);
        *connected_ = true;
        return connection{std::weak_ptr{connected_}};
    }

    template <auto Func, class = std::enable_if_t<
        std::is_invocable_v<decltype(Func), Args...> ||
        std::is_member_function_pointer_v<decltype(Func)>>>
    std::weak_ptr<bool> connect() {
        if constexpr (std::is_invocable_v<decltype(Func), Args...>)
            return connect(Func);
        else {
            using clazz = typename reflect::function_traits<decltype(Func)>::clazz;
            return connect([obj = clazz{}] (Args&&...args) mutable {
                return std::invoke(Func, obj, std::forward<Args>(args)...);
            });
        }
    }

    template <auto Func, class T, class = std::enable_if_t<
        std::is_invocable_v<decltype(Func), T&, Args...>>>
    connection connect(T&& obj) {
        return connect([obj = std::forward<T>(obj)](Args&&...args) mutable {
            return std::invoke(Func, obj, std::forward<Args>(args)...);
        });
    }

    template <auto Func, class T, class = std::enable_if_t<
        std::is_invocable_v<decltype(Func), T*, Args...>>>
    connection connect(T* obj) {
        return connect([=](Args&&...args) mutable {
            return std::invoke(Func, obj, std::forward<Args>(args)...);
        });
    }

    template <auto Func, class T, class = std::enable_if_t<
        std::is_invocable_v<decltype(Func), T&, Args...>>>
    connection connect_shared(const std::shared_ptr<T>& p) {
        return connect([=](Args&&...args) mutable {
            return std::invoke(Func, *p, std::forward<Args>(args)...);
        });
    }

private:
    std::function<Fn> call_;
    std::shared_ptr<bool> connected_{ new bool{} };

};

}
