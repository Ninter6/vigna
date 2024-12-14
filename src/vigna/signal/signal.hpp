//
// Created by Ninter6 on 2024/12/10.
//

#pragma once

#include "delegate.hpp"
#include "vigna/core/dense_map.hpp"

namespace vigna {

template <class Delegate, class Alloc = std::allocator<Delegate>>
class signal {
     static_assert(std::is_same_v<Delegate, typename Alloc::value_type>);
     using container = std::vector<Delegate, Alloc>;

public:
     signal() = default;

private:
     container calls_;

};

}
