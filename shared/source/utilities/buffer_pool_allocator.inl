/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/memory_manager/memory_manager.h"
#include "shared/source/utilities/buffer_pool_allocator.h"
#include "shared/source/utilities/heap_allocator.h"

#include <type_traits>

namespace NEO {

template <typename PoolT, typename BufferType, typename BufferParentType>
AbstractBuffersPool<PoolT, BufferType, BufferParentType>::AbstractBuffersPool(MemoryManager *mm) : memoryManager{mm} {
    static_assert(std::is_base_of_v<BufferParentType, BufferType>);
}

template <typename PoolT, typename BufferType, typename BufferParentType>
AbstractBuffersPool<PoolT, BufferType, BufferParentType>::AbstractBuffersPool(AbstractBuffersPool<PoolT, BufferType, BufferParentType> &&bufferPool)
    : memoryManager{bufferPool.memoryManager},
      mainStorage{std::move(bufferPool.mainStorage)},
      chunkAllocator{std::move(bufferPool.chunkAllocator)} {}

template <typename PoolT, typename BufferType, typename BufferParentType>
void AbstractBuffersPool<PoolT, BufferType, BufferParentType>::tryFreeFromPoolBuffer(BufferParentType *possiblePoolBuffer, size_t offset, size_t size) {
    if (this->isPoolBuffer(possiblePoolBuffer)) {
        this->chunksToFree.push_back({offset + startingOffset, size});
    }
}

template <typename PoolT, typename BufferType, typename BufferParentType>
bool AbstractBuffersPool<PoolT, BufferType, BufferParentType>::isPoolBuffer(const BufferParentType *buffer) const {
    static_assert(std::is_base_of_v<BufferParentType, BufferType>);

    return (buffer && this->mainStorage.get() == buffer);
}

template <typename PoolT, typename BufferType, typename BufferParentType>
void AbstractBuffersPool<PoolT, BufferType, BufferParentType>::drain() {
    const auto &allocationsVec = this->getAllocationsVector();
    for (auto allocation : allocationsVec) {
        if (allocation && this->memoryManager->allocInUse(*allocation)) {
            return;
        }
    }
    for (auto &chunk : this->chunksToFree) {
        this->chunkAllocator->free(chunk.first, chunk.second);
    }
    this->chunksToFree.clear();
}

template <typename BuffersPoolType, typename BufferType, typename BufferParentType>
bool AbstractBuffersAllocator<BuffersPoolType, BufferType, BufferParentType>::isPoolBuffer(const BufferParentType *buffer) const {
    static_assert(std::is_base_of_v<BufferParentType, BufferType>);

    for (auto &bufferPool : this->bufferPools) {
        if (bufferPool.isPoolBuffer(buffer)) {
            return true;
        }
    }
    return false;
}

template <typename BuffersPoolType, typename BufferType, typename BufferParentType>
void AbstractBuffersAllocator<BuffersPoolType, BufferType, BufferParentType>::tryFreeFromPoolBuffer(BufferParentType *possiblePoolBuffer, size_t offset, size_t size) {
    auto lock = std::unique_lock<std::mutex>(this->mutex);
    for (auto &bufferPool : this->bufferPools) {
        bufferPool.tryFreeFromPoolBuffer(possiblePoolBuffer, offset, size); // NOLINT(clang-analyzer-cplusplus.NewDelete)
    }
}

template <typename BuffersPoolType, typename BufferType, typename BufferParentType>
void AbstractBuffersAllocator<BuffersPoolType, BufferType, BufferParentType>::drain() {
    for (auto &bufferPool : this->bufferPools) {
        bufferPool.drain();
    }
}

template <typename BuffersPoolType, typename BufferType, typename BufferParentType>
void AbstractBuffersAllocator<BuffersPoolType, BufferType, BufferParentType>::addNewBufferPool(BuffersPoolType &&bufferPool) {
    if (bufferPool.mainStorage) {
        this->bufferPools.push_back(std::move(bufferPool));
    }
}
} // namespace NEO