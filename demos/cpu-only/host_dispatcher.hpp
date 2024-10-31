#pragma once

#include "core/thread_pool.hpp"
#include "shared/structures.h"
#include <memory>

namespace cpu {

core::multi_future<void> dispatch_MortonCode(
    core::thread_pool& pool,
    int num_threads,
    const std::shared_ptr<const Pipe>& p);

void dispatch_RadixSort(
    core::thread_pool& pool,
    int num_threads,
    const std::shared_ptr<const Pipe>& p);

}  // namespace cpu
