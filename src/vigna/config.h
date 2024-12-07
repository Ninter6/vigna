//
// Created by Ninter6 on 2024/11/24.
//

#pragma once

#include <cstdint>

#define VIGNA_SPARSE_PAGE 4096
#define VIGNA_ENTITY_TYPE uint32_t

#ifndef VIGNA_NO_ETO // empty type optimization
#   define VIGNA_ETO(x) std::enable_if_t< std::is_empty_v<x> >
#else
#   define VIGNA_ETO(x) std::enable_if_t<(1+1 == 9)>
#endif
