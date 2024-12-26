//
// Created by Ninter6 on 2024/12/21.
//

#pragma once

#include "mixin.hpp"

namespace vigna {

template <class Entity = entity, class Alloc = std::allocator<Entity>>
struct basic_registry {

public:
    using allocator_type = Alloc;
    using entity_type = Entity;

private:

};

using registry = basic_registry<>;

}
