#pragma once

#include <memory>

#include "core/thread_pool.hpp"
#include "shared/structures.h"
#include "third-party/BS_thread_pool.hpp"

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

// ----------------------------------------------------------------------------

void dispatch_MortonCode(BS::thread_pool& pool,
                         int num_threads,
                         const std::shared_ptr<const Pipe>& p);

void dispatch_RadixSort(BS::thread_pool& pool,
                        int num_threads,
                        const std::shared_ptr<const Pipe>& p);
                        
void dispatch_RemoveDuplicates(BS::thread_pool& pool,
                               int num_threads,
                               const std::shared_ptr<Pipe>& p);

void dispatch_BuildRadixTree(BS::thread_pool& pool,
                             int num_threads,
                             const std::shared_ptr<const Pipe>& p);

void dispatch_EdgeCount(BS::thread_pool& pool,
                        int num_threads,
                        const std::shared_ptr<const Pipe>& p);

void dispatch_EdgeOffset(BS::thread_pool& pool,
                         int num_threads,
                         const std::shared_ptr<const Pipe>& p);

void dispatch_BuildOctree(BS::thread_pool& pool,
                          int num_threads,
                          const std::shared_ptr<const Pipe>& p);

}  // namespace cpu
