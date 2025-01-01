#include <iostream>
#include <set>
#include <list>
#include <ranges>
#include <source_location>

#include <vigna.hpp>

int main() {
    std::cout << "Hello, World!" << std::endl;

    vigna::dense_map<int, float> map;
    map[114] = 514.f;

    vigna::dense_map<int, float>::const_iterator it = map.begin();

    std::cout << std::as_const(map)[114] << std::endl;

    using traits = vigna::entity_traits<vigna::entity>;
    vigna::entity entity = vigna::null;
    std::cout << std::boolalpha << (traits::value(entity)) << std::endl;

    vigna::basic_sparse_set<vigna::entity> sparse_set;
    sparse_set.push(traits::construct(5, 114));
    sparse_set.push(traits::construct(24, 0));
    sparse_set.push(traits::construct(3, 6));
    sparse_set.push(traits::construct(35, 0));
    sparse_set.emplace(13, 77);
    sparse_set.sort();
    std::cout << traits::id(*sparse_set.find(traits::construct(35, 51))) << std::endl;

    vigna::basic_storage<vigna::entity, int> storage;
    auto e = traits::construct(5, 1);
    auto e1 = traits::combine(sparse_set.back(), e);
    storage.emplace(e1, 1919);
    storage[e] = 114514;
    storage.erase(storage.find(e1));
    std::cout << storage.get(e) << " " << storage.contains(e1) << std::endl;

    int arr[]{1, 1, 4, 5, 1, 4};
    for (auto&& i : arr | vigna::view::all)
        std::cout << i;
    std::endl(std::cout);

    std::list list{1, 1, 4, 5, 1, 4};
    for (auto&& i : list | vigna::view::all() | vigna::view::take(4))
        std::cout << i;
    std::cout << std::endl;

    auto view = vigna::view::pack(vigna::view::iota() | vigna::view::take(4), arr) | vigna::view::transform([](auto&& i) { return std::get<1>(i); }) | vigna::view::common;
    std::copy(view.begin(), view.end(), std::ostream_iterator<int>(std::cout, " "));
    std::endl(std::cout);

    vigna::basic_storage<vigna::entity, vigna::entity> entities;
    auto e2 = traits::id(entities.emplace());
    std::cout << e2 << std::endl;

    using tl = vigna::reflect::type_list<int, float, char, std::string, double>;
    vigna::reflect::type_list_element_t<3, tl> str = "114514";
    std::cout << str << ' ' << vigna::reflect::type_list_find_v<std::string, tl> << std::endl;

    auto call = [](int a, int b) { std::cout << "signal: " << a << ' ' << b << '\n'; };
    vigna::signal<vigna::delegate<int, int>> signal;
    auto conn = signal.connect<&decltype(call)::operator()>(&call);
    signal.emit(114, 514);
    conn.release();
    signal.emit(1919, 810);

    vigna::registry registry;
    vigna::basic_signal_mixin<decltype(entities), vigna::registry> mixin{&registry};
    auto conn1 = mixin.on_construct().connect([](auto&& r, auto&& e) { std::cout << (int)traits::version(e); });
    mixin.emplace(traits::construct(0, 1919));
    mixin.emplace(traits::construct(0, 514));
    conn1.release();
    mixin.emplace(traits::construct(1, 514));
    std::endl(std::cout);

    auto e3 = registry.create(traits::construct(0, 810));
    std::cout << traits::version(e3) << std::endl;

    for (auto&& i : arr | vigna::view::filter([](auto&& i) { return i != 1; }))
        std::cout << i;
    std::endl(std::cout);

    registry.emplace<int>(e3, 114514);
    registry.emplace<double>(e3, 114.514);
    registry.emplace_or_replace<int>(e3, 1919810);

    auto e4 = registry.create();
    registry.emplace<int>(e4, 1919);
    registry.emplace<double>(e4, 114514);
    registry.patch<double>(e4, [](auto&& v) { v = 8.10; });

    auto view1 = std::as_const(registry).view<int, double>();
    for (auto&& [vigna, i, d] : view1.each()) {
        std::cout << "entity[" << traits::id(vigna) << "]: " << i << ' ' << d << '\n';
    }

    view1.for_each([](auto&& e, auto&&...args) {
        ((std::cout << args << ' '), ...);
        std::cout << "from entity[" << traits::id(e) << "]\n";
    });

    return 0;
}
