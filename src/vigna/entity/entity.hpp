//
// Created by Ninter6 on 2024/11/24.
//

#pragma once

#include "vigna/config.h"

namespace vigna {

namespace detail {

template <typename T>
constexpr size_t bit_width(T num) {
    static_assert(std::is_unsigned<T>::value, "bit_width requires an unsigned type");
    size_t width = 0;
    if (num) do ++width; while ((num >>= 1) != 0);
    return width;
}

template <class T, class = void>
struct entity_traits {};

template <class T>
struct entity_traits<T, std::enable_if_t<std::is_enum_v<T>>> : entity_traits<std::underlying_type_t<T>> {
    using entity_type = T;
    static constexpr entity_type null = static_cast<T>(entity_traits<std::underlying_type_t<T>>::null);
};

template <>
struct entity_traits<uint16_t> {
    using entity_type = uint16_t;
    using value_type = uint16_t;
    using id_type = uint8_t;
    using version_type = uint8_t;
    
    static constexpr id_type id_max = 255;
    static constexpr version_type version_max = 255;
    static constexpr entity_type null = ~0;
};

template <>
struct entity_traits<uint32_t> {
    using entity_type = uint32_t;
    using value_type = uint32_t;
    using id_type = uint32_t; // 24
    using version_type = uint8_t; // 8

    static constexpr id_type id_max = 16777215u;
    static constexpr version_type version_max = 255u;
    static constexpr entity_type null = ~0u;
};

template <>
struct entity_traits<uint64_t> {
    using entity_type = uint64_t;
    using value_type = uint64_t;
    using id_type = uint32_t;
    using version_type = uint32_t;

    static constexpr id_type id_max = 0xffffffffu;
    static constexpr version_type version_max = 0xffffffffu;
    static constexpr entity_type null = ~0u;
};

} // detail

template <class T>
struct entity_traits : detail::entity_traits<T> {
    using base_type = detail::entity_traits<T>;

    using entity_type = typename base_type::entity_type;
    using value_type = typename base_type::value_type;
    using id_type = typename base_type::id_type;
    using version_type = typename base_type::version_type;

    static constexpr entity_type null = base_type::null;
    
    static constexpr id_type id_max = base_type::id_max;
    static constexpr version_type version_max = base_type::version_max;

    static constexpr value_type id_mask = static_cast<value_type>(id_max);
    static constexpr value_type version_mask = ~id_mask;

    static constexpr size_t version_bise = detail::bit_width(id_mask);

    static constexpr entity_type construct(id_type id, version_type version) {
        return static_cast<entity_type>(((version << version_bise) & version_mask) | (id & id_mask));
    }

    static constexpr entity_type construct(value_type val) {
        return static_cast<entity_type>(val);
    }

    static constexpr id_type id(entity_type entity) {
        return static_cast<id_type>(static_cast<value_type>(entity) & id_mask);
    }

    static constexpr version_type version(entity_type entity) {
        return static_cast<version_type>((static_cast<value_type>(entity) & version_mask) >> version_bise);
    }

    static constexpr value_type value(entity_type entity) {
        return static_cast<value_type>(entity);
    }

    static constexpr entity_type combine(entity_type e_id, entity_type e_ver) {
        return construct(id(e_id), version(e_ver));
    }

    static constexpr id_type next_id(entity_type entity) {
        auto n = id(entity);
        if (++n == id_max) return 0;
        return n;
    }
    
    static constexpr id_type next_version(entity_type entity) {
        auto n = version(entity);
        if (++n == version_max) return 0;
        return n;
    }

    static constexpr entity_type next(entity_type entity) {
        return construct(next_id(entity), next_version(entity));
    }

    static constexpr entity_type add_id(entity_type entity) {
        return construct(next_id(entity), version(entity));
    }
    
    static constexpr entity_type add_version(entity_type entity) {
        return construct(id(entity), next_version(entity));
    }
};

using entity_underlying_type = VIGNA_ENTITY_TYPE;

enum class entity : entity_underlying_type {};

struct null_t {
    template <class T, class traits = entity_traits<T>>
    operator T() const { return traits::null; } // NOLINT(*-explicit-constructor)

    template <class T, class traits = entity_traits<T>>
    bool operator==(const T& entity) const { return entity == traits::null; }

    template <class T, class traits = entity_traits<T>>
    bool operator!=(const T& entity) const { return entity != traits::null; }

    template <class T, class traits = entity_traits<T>>
    friend bool operator==(const T& entity, const null_t&) { return entity == traits::null; }

    template <class T, class traits = entity_traits<T>>
    friend bool operator!=(const T& entity, const null_t&) { return entity != traits::null; }
};

constexpr null_t null;

} // namespace vigna
