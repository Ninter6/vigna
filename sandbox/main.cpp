#include <iostream>
#include <set>
#include <list>
#include <ranges>

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
    storage.emplace_or_replace(e, 114514);
    auto e1 = traits::combine(sparse_set.back(), e);
    storage.emplace(e1, 1919);
    storage.pop(e1);
    std::cout << storage[e] << " " << storage.contains(e1) << std::endl;

    int arr[]{1, 1, 4, 5, 1, 4};
    for (auto&& i : arr | vigna::view::all)
        std::cout << i;
    std::endl(std::cout);

    std::list list{1, 1, 4, 5, 1, 4};
    for (auto&& i : list | vigna::view::take(6))
        std::cout << i;
    std::cout << std::endl;

    auto view = vigna::view::pack(vigna::view::iota() | vigna::view::take(4), arr) | vigna::view::transform([](auto&& i) { return std::get<1>(i); });
    std::copy(view.begin(), view.end(), std::ostream_iterator<int>(std::cout, " "));
    std::endl(std::cout);

    return 0;
}
