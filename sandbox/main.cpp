#include <iostream>

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
    sparse_set.emplace();
    sparse_set.sort();
    std::cout << traits::id(*sparse_set.find(traits::construct(35, 51))) << std::endl;

    return 0;
}
