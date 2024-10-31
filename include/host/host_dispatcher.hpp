#pragma once

#include <memory>

#include "core/thread_pool.hpp"
#include "shared/structures.h"

namespace cpu {

void dispatch_MortonCode(core::thread_pool& pool,
                         int num_threads,
                         const std::shared_ptr<const Pipe>& p);

void dispatch_RadixSort(core::thread_pool& pool,
                        int num_threads,
                        const std::shared_ptr<const Pipe>& p);

void dispatch_RemoveDuplicates(core::thread_pool& pool,
                               int num_threads,
                               const std::shared_ptr<Pipe>& p);

void dispatch_BuildRadixTree(core::thread_pool& pool,
                             int num_threads,
                             const std::shared_ptr<const Pipe>& p);

void dispatch_EdgeCount(core::thread_pool& pool,
                        int num_threads,
                        const std::shared_ptr<const Pipe>& p);

void dispatch_EdgeOffset(core::thread_pool& pool,
                         int num_threads,
                         const std::shared_ptr<const Pipe>& p);

void dispatch_BuildOctree(core::thread_pool& pool,
                          int num_threads,
                          const std::shared_ptr<const Pipe>& p);

}  // namespace cpu
