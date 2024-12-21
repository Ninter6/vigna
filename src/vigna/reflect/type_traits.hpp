//
// Created by Ninter6 on 2024/12/16.
//

#pragma once

#include <memory>

#include "utility.hpp"

namespace vigna::reflect {

namespace detail {

struct basic_type_guard_inner {
    virtual ~basic_type_guard_inner() = default;

    [[nodiscard]] virtual size_t hash() const = 0;
    [[nodiscard]] virtual basic_type_guard_inner* clone() const = 0;
    [[nodiscard]] virtual bool accept(const basic_type_guard_inner*) const = 0;
};

template <class...Args>
struct type_guard_inner : basic_type_guard_inner {
    using type = type_guard_inner;

    type_guard_inner() = default;

    [[nodiscard]] size_t hash() const override {
        size_t v = 0;
        auto cmb = [&] (size_t h) { v ^= h + 0x9e3779b9 + (v << 6) + (v >> 2); };
        return (cmb(typeid(Args).hash_code()), ...), v;
    }

    [[nodiscard]] basic_type_guard_inner* clone() const override {
        return new type{};
    }

    [[nodiscard]] bool accept(const basic_type_guard_inner* other) const override {
        return dynamic_cast<const type*>(other) != nullptr;
    }
};

}

struct type_guard {
    type_guard(type_guard&&) = default;
    type_guard& operator=(type_guard&&) = default;

    type_guard(const type_guard& o) : inner_(o.inner_->clone()) {}
    type_guard& operator=(const type_guard& o) {
        if (*this != o) inner_.reset(o.inner_->clone());
        return *this;
    }

    bool operator==(const type_guard& o) const { return inner_->accept(o.inner_.get()); }
    bool operator!=(const type_guard& o) const { return !(*this == o); }

private:
    template <class...Args>
    friend type_guard make_type_guard();
    friend std::hash<type_guard>;

    explicit type_guard(std::unique_ptr<detail::basic_type_guard_inner>&& inner)
        : inner_(std::move(inner)) {}
    std::unique_ptr<detail::basic_type_guard_inner> inner_;
};

template <class...Args>
[[nodiscard]] type_guard make_type_guard() {
    return type_guard{std::make_unique<detail::type_guard_inner<Args...>>()};
}

}

template <>
struct std::hash<vigna::reflect::type_guard> {
    size_t operator()(const vigna::reflect::type_guard& o) const noexcept {
        return o.inner_->hash();
    }
};
