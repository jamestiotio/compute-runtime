/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/constants.h"
#include "shared/source/utilities/stackvec.h"

#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <vector>

namespace NEO {

class GraphicsAllocation;
class HeapAllocator;
class MemoryManager;

template <typename PoolT>
struct SmallBuffersParams {
  protected:
    static constexpr auto aggregatedSmallBuffersPoolSize = 64 * KB;
    static constexpr auto smallBufferThreshold = 4 * KB;
    static constexpr auto chunkAlignment = 512u;
    static constexpr auto startingOffset = chunkAlignment;
};

template <typename PoolT, typename BufferType, typename BufferParentType = BufferType>
struct AbstractBuffersPool : public SmallBuffersParams<PoolT> {
    // The prototype of a function allocating the `mainStorage` is not specified.
    // That would be an unnecessary limitation here - it is completely up to derived class implementation.
    // Perhaps the allocating function needs to leverage `HeapAllocator::allocate()` and also
    // a BufferType-dependent function reserving chunks within `mainStorage`.
    // Example: see `NEO::Context::BufferPool::allocate()`
    using Params = SmallBuffersParams<PoolT>;
    using Params::aggregatedSmallBuffersPoolSize;
    using Params::chunkAlignment;
    using Params::smallBufferThreshold;
    using Params::startingOffset;
    using AllocsVecCRef = const StackVec<NEO::GraphicsAllocation *, 1> &;

    AbstractBuffersPool(MemoryManager *memoryManager);
    AbstractBuffersPool(AbstractBuffersPool<PoolT, BufferType, BufferParentType> &&bufferPool);
    void tryFreeFromPoolBuffer(BufferParentType *possiblePoolBuffer, size_t offset, size_t size);
    bool isPoolBuffer(const BufferParentType *buffer) const;
    void drain();

    // Derived class needs to provide its own implementation of getAllocationsVector().
    // This is a CRTP-replacement for virtual functions.
    AllocsVecCRef getAllocationsVector() {
        return static_cast<PoolT *>(this)->getAllocationsVector();
    }

    MemoryManager *memoryManager{nullptr};
    std::unique_ptr<BufferType> mainStorage;
    std::unique_ptr<HeapAllocator> chunkAllocator;
    std::vector<std::pair<uint64_t, size_t>> chunksToFree;
};

template <typename BuffersPoolType, typename BufferType, typename BufferParentType = BufferType>
class AbstractBuffersAllocator : public SmallBuffersParams<BuffersPoolType> {
    // The prototype of a function allocating buffers from the pool is not specified (see similar comment in `AbstractBufersPool`).
    // By common sense, in order to allocate buffers from the pool the function should leverage a call provided by `BuffersPoolType`.
    // Example: see `NEO::Context::BufferPoolAllocator::allocateBufferFromPool()`.
  public:
    using Params = SmallBuffersParams<BuffersPoolType>;
    using Params::aggregatedSmallBuffersPoolSize;
    using Params::chunkAlignment;
    using Params::smallBufferThreshold;
    using Params::startingOffset;
    static_assert(aggregatedSmallBuffersPoolSize > smallBufferThreshold, "Largest allowed buffer needs to fit in pool");

    void releaseSmallBufferPool() { this->bufferPools.clear(); }
    bool isPoolBuffer(const BufferParentType *buffer) const;
    void tryFreeFromPoolBuffer(BufferParentType *possiblePoolBuffer, size_t offset, size_t size);

  protected:
    inline bool isSizeWithinThreshold(size_t size) const { return smallBufferThreshold >= size; }
    void drain();
    void addNewBufferPool(BuffersPoolType &&bufferPool);

    std::mutex mutex;
    std::vector<BuffersPoolType> bufferPools;
};
} // namespace NEO
