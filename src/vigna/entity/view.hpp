//
// Created by Ninter6 on 2024/12/31.
//

#pragma once

#include <array>

#include "vigna/range/view.hpp"
#include "vigna/reflect/utility.hpp"

namespace vigna {

template <class...Args>
using get_t = reflect::type_list<Args...>;

template <class...Args>
using exclude_t = reflect::type_list<Args...>;

template <class, class, class>
class basic_view;

template <class T, size_t Get, size_t Exclude>
class basic_common_view {

    auto get_iterable() const {
        const auto& iterable = index != Get ? (*get(index) | view::all) : range::subrange<typename T::iterator>{};
        return view::filter(iterable, [&](auto&& e) { return contains(e); });
    }

protected:
    void use(size_t i) {
        assert(i < Get && "out of range");
        index = i;
    }

    T* get(size_t i) const {
        return get_.at(i);
    }

public:
    using common_type = T;
    using entity_type = typename T::entity_type;

    basic_common_view() = default;
    basic_common_view(const std::array<common_type*, Get>& get, const std::array<common_type*, Exclude>& exclude)
        : get_(get), exclude_(exclude) { refresh(); }

    auto begin() const { return get_iterable().begin(); }
    auto end() const { return get_iterable().end(); }

    void refresh() {
        index = 0;
        if (Get < 2) return;
        for (size_t i = 0; i < Get; ++i) {
            if (get_[i] == nullptr) {
                index = Get;
                return;
            }
            if (get_[i]->size() < get_[index]->size())
                index = i;
        }
    }

    bool contains(const entity_type& entity) const {
        const auto f = [entity](auto&& p) { return p && p->contains(entity); };
        return index < Get &&
            std::all_of(get_.begin(), get_.end(), f) &&
            std::none_of(exclude_.begin(), exclude_.end(), f);
    }

private:
    size_t index{Get};
    std::array<common_type*, Get> get_;
    std::array<common_type*, Exclude> exclude_;

};

template <class Common, class...Get, class...Exclude>
class basic_view<Common, get_t<Get...>, exclude_t<Exclude...>> : basic_common_view<Common, sizeof...(Get), sizeof...(Exclude)> {
    using base_type = basic_common_view<Common, sizeof...(Get), sizeof...(Exclude)>;
    using common_type = Common;

    using get_list = get_t<Get...>;
    using exclude_list = exclude_t<Exclude...>;

    template <class T>
    static constexpr size_t index_of = reflect::type_list_find_v<T, reflect::type_list<typename Get::element_type...>>;

    template <size_t I>
    auto get_as_tuple(const typename base_type::entity_type& entity) const {
        if constexpr (std::is_void_v<decltype(get<I>(entity))>)
            return std::forward_as_tuple();
        else
            return std::forward_as_tuple(get<I>(entity));
    }

public:
    using typename base_type::common_type;
    using typename base_type::entity_type;

    basic_view() = default;
    explicit basic_view(Get*...get, Exclude*...exclude)
        : base_type({get...}, {exclude...}) {}

    template <class T>
    void sort_as() { sort_as<index_of<T>>(); }

    template <size_t I>
    void sort_as() {
        static_assert(I < get_list::size, "Invalid type");
        base_type::use(I);
    }

    template <class T>
    auto get() const { return get<index_of<T>>(); }

    template <size_t I>
    auto get() const {
        static_assert(I < get_list::size, "Invalid type");
        return static_cast<reflect::type_list_element_t<I, get_list>*>(base_type::get(I));
    }

    template <class T>
    decltype(auto) get(const entity& entity) const { return get<index_of<T>>(entity); }

    template <size_t I>
    decltype(auto) get(const entity& entity) const {
        assert(get<I>() != nullptr);
        return get<I>()->get(entity);
    }

    using base_type::begin;
    using base_type::end;

    using base_type::contains;

    auto each() const {
        return view::transform(*this, [&](auto&& e) {
            return std::tuple_cat(std::make_tuple(e),
                                  get_as_tuple<index_of<typename Get::element_type>>(e)...);
        });
    }

    template<class Fn, class = std::enable_if_t<
        std::is_invocable_v<Fn, entity_type, typename Get::element_type...>>>
    auto for_each(Fn&& fn) const {
        return range::for_each(each(), [&](auto&& tp) {
            std::apply(std::forward<Fn>(fn), tp);
        });
    }

};

}
