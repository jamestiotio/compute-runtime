/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/utilities/buffer_pool_allocator.inl"
#include "shared/source/utilities/heap_allocator.h"
#include "shared/source/utilities/stackvec.h"
#include "shared/test/common/mocks/mock_execution_environment.h"
#include "shared/test/common/mocks/mock_memory_manager.h"

#include "gtest/gtest.h"

#include <memory>

struct DummyBufferPool;

struct DummyBuffer {
    DummyBuffer(int v) : val{v} {}
    int val;
};

template <>
struct NEO::SmallBuffersParams<DummyBufferPool> {
    static constexpr auto aggregatedSmallBuffersPoolSize = 32 * KB;
    static constexpr auto smallBufferThreshold = 2 * KB;
    static constexpr auto chunkAlignment = 1024u;
    static constexpr auto startingOffset = chunkAlignment;
};

struct DummyBuffersPool : public NEO::AbstractBuffersPool<DummyBuffersPool, DummyBuffer> {
    using BaseType = NEO::AbstractBuffersPool<DummyBuffersPool, DummyBuffer>;
    static constexpr auto dummyPtr = 0xdeadbeef0000;

    DummyBuffersPool(NEO::MemoryManager *memoryManager, uint32_t poolOffset) : BaseType(memoryManager) {
        dummyAllocations.resize(2);
        dummyAllocations[0] = reinterpret_cast<NEO::GraphicsAllocation *>(poolOffset + dummyPtr);
        dummyAllocations[1] = nullptr; // makes sure nullptrs don't cause SEGFAULTs
    }

    DummyBuffersPool(NEO::MemoryManager *memoryManager) : DummyBuffersPool(memoryManager, 0x0) {}

    BaseType::AllocsVecCRef getAllocationsVector() {
        return dummyAllocations;
    }

    StackVec<NEO::GraphicsAllocation *, 1> dummyAllocations;
};

struct DummyBuffersAllocator : public NEO::AbstractBuffersAllocator<DummyBuffersPool, DummyBuffer> {
    using BaseType = NEO::AbstractBuffersAllocator<DummyBuffersPool, DummyBuffer>;
    using BaseType::addNewBufferPool;
    using BaseType::bufferPools;
    using BaseType::drain;
    using BaseType::isSizeWithinThreshold;
};

using NEO::MockExecutionEnvironment;
using NEO::MockMemoryManager;

struct AbstractSmallBuffersTest : public ::testing::Test {
    void SetUp() override {
        this->memoryManager.reset(new MockMemoryManager{this->executionEnvironment});
    }

    MockExecutionEnvironment executionEnvironment;
    std::unique_ptr<MockMemoryManager> memoryManager;
    static constexpr auto testVal = 0x1234;
};

TEST_F(AbstractSmallBuffersTest, givenBuffersPoolWhenCreatedAndMovedThenCtorsWorkCorrectly) {
    auto pool1 = DummyBuffersPool{this->memoryManager.get()};
    pool1.mainStorage.reset(new DummyBuffer(testVal));
    pool1.chunkAllocator.reset(new NEO::HeapAllocator{DummyBuffersPool::startingOffset,
                                                      DummyBuffersPool::aggregatedSmallBuffersPoolSize,
                                                      DummyBuffersPool::chunkAlignment,
                                                      DummyBuffersPool::smallBufferThreshold});

    EXPECT_EQ(pool1.memoryManager, this->memoryManager.get());

    auto pool2 = std::move(pool1);
    EXPECT_EQ(pool2.memoryManager, this->memoryManager.get());
    EXPECT_EQ(pool2.mainStorage->val, testVal);
    EXPECT_EQ(static_cast<DummyBuffersPool::BaseType &>(pool2).getAllocationsVector()[0], reinterpret_cast<NEO::GraphicsAllocation *>(DummyBuffersPool::dummyPtr));
    EXPECT_EQ(pool2.chunkAllocator->getUsedSize(), 0ul);
    EXPECT_EQ(pool2.chunkAllocator->getLeftSize(), DummyBuffersPool::aggregatedSmallBuffersPoolSize);
}

TEST_F(AbstractSmallBuffersTest, givenBuffersAllocatorWhenPoolWithoutMainStorageAddedThenItIsIgnored) {
    auto pool = DummyBuffersPool{this->memoryManager.get()};
    pool.mainStorage.reset(nullptr);
    auto buffersAllocator = DummyBuffersAllocator{};
    buffersAllocator.addNewBufferPool(std::move(pool));

    EXPECT_EQ(buffersAllocator.bufferPools.size(), 0u);
}

TEST_F(AbstractSmallBuffersTest, givenBuffersAllocatorWhenNullptrTriedToBeFreedThenItIsNotConsideredValidBuffer) {
    auto pool = DummyBuffersPool{this->memoryManager.get()};
    pool.mainStorage.reset(new DummyBuffer(testVal));
    auto buffersAllocator = DummyBuffersAllocator{};
    buffersAllocator.addNewBufferPool(std::move(pool));

    EXPECT_TRUE(buffersAllocator.isSizeWithinThreshold(DummyBuffersPool::smallBufferThreshold));
    EXPECT_FALSE(buffersAllocator.isSizeWithinThreshold(DummyBuffersPool::smallBufferThreshold + 1));

    auto &chunksToFree = buffersAllocator.bufferPools[0].chunksToFree;
    EXPECT_EQ(chunksToFree.size(), 0u);
    pool.tryFreeFromPoolBuffer(nullptr, 0x42, 42);
    EXPECT_EQ(chunksToFree.size(), 0u);
}

TEST_F(AbstractSmallBuffersTest, givenBuffersAllocatorWhenNonMainStorageTriedToBeFreedThenItIsNotRegisteredForFreeing) {
    auto pool = DummyBuffersPool{this->memoryManager.get()};
    pool.mainStorage.reset(new DummyBuffer(testVal));
    auto buffersAllocator = DummyBuffersAllocator{};
    buffersAllocator.addNewBufferPool(std::move(pool));
    auto otherBuffer = std::make_unique<DummyBuffer>(888);

    auto &chunksToFree = buffersAllocator.bufferPools[0].chunksToFree;
    EXPECT_EQ(chunksToFree.size(), 0u);
    buffersAllocator.tryFreeFromPoolBuffer(otherBuffer.get(), 0x88, 0x400);
    EXPECT_EQ(chunksToFree.size(), 0u);
}

TEST_F(AbstractSmallBuffersTest, givenBuffersAllocatorWithMultiplePoolsWhenSearchingForContributingBufferThenItIsFound) {
    auto pool1 = DummyBuffersPool{this->memoryManager.get()};
    auto pool2 = DummyBuffersPool{this->memoryManager.get()};
    pool1.mainStorage.reset(new DummyBuffer(testVal));
    pool2.mainStorage.reset(new DummyBuffer(testVal + 2));
    auto buffer1 = pool1.mainStorage.get();
    auto buffer2 = pool2.mainStorage.get();
    auto otherBuffer = std::make_unique<DummyBuffer>(888);

    auto buffersAllocator = DummyBuffersAllocator{};
    buffersAllocator.addNewBufferPool(std::move(pool1));
    buffersAllocator.addNewBufferPool(std::move(pool2));

    EXPECT_TRUE(buffersAllocator.isPoolBuffer(buffer1));
    EXPECT_TRUE(buffersAllocator.isPoolBuffer(buffer2));
    EXPECT_FALSE(buffersAllocator.isPoolBuffer(otherBuffer.get()));
}

TEST_F(AbstractSmallBuffersTest, givenBuffersAllocatorWhenChunkOfMainStorageTriedToBeFreedThenItIsEnlistedToBeFreed) {
    auto pool1 = DummyBuffersPool{this->memoryManager.get()};
    auto pool2 = DummyBuffersPool{this->memoryManager.get()};
    pool1.mainStorage.reset(new DummyBuffer(testVal));
    pool2.mainStorage.reset(new DummyBuffer(testVal + 2));
    auto poolStorage2 = pool2.mainStorage.get();

    auto buffersAllocator = DummyBuffersAllocator{};
    buffersAllocator.addNewBufferPool(std::move(pool1));
    buffersAllocator.addNewBufferPool(std::move(pool2));

    auto &chunksToFree1 = buffersAllocator.bufferPools[0].chunksToFree;
    auto &chunksToFree2 = buffersAllocator.bufferPools[1].chunksToFree;
    EXPECT_EQ(chunksToFree1.size(), 0u);
    EXPECT_EQ(chunksToFree2.size(), 0u);
    auto chunkSize = sizeof(DummyBuffer) / 8;
    auto chunkOffset = sizeof(DummyBuffer) / 2;
    buffersAllocator.tryFreeFromPoolBuffer(poolStorage2, chunkOffset, chunkSize);
    EXPECT_EQ(chunksToFree1.size(), 0u);
    EXPECT_EQ(chunksToFree2.size(), 1u);
    auto [effectiveChunkOffset, size] = chunksToFree2[0];
    EXPECT_EQ(effectiveChunkOffset, chunkOffset + DummyBuffersPool::startingOffset);
    EXPECT_EQ(size, chunkSize);

    buffersAllocator.releaseSmallBufferPool();
    EXPECT_EQ(buffersAllocator.bufferPools.size(), 0u);
}

TEST_F(AbstractSmallBuffersTest, givenBuffersAllocatorWhenDrainingPoolsThenOnlyAPoolWithoutAllocationsInUseIsDrained) {
    auto otherMemoryManager = std::make_unique<MockMemoryManager>(this->executionEnvironment);

    auto pool1 = DummyBuffersPool{this->memoryManager.get()};
    auto pool2 = DummyBuffersPool{otherMemoryManager.get()};
    pool1.mainStorage.reset(new DummyBuffer(testVal));
    pool2.mainStorage.reset(new DummyBuffer(testVal + 2));
    auto buffer1 = pool1.mainStorage.get();
    auto buffer2 = pool2.mainStorage.get();
    pool1.chunkAllocator.reset(new NEO::HeapAllocator{DummyBuffersPool::startingOffset,
                                                      DummyBuffersPool::aggregatedSmallBuffersPoolSize,
                                                      DummyBuffersPool::chunkAlignment,
                                                      DummyBuffersPool::smallBufferThreshold});
    pool2.chunkAllocator.reset(new NEO::HeapAllocator{DummyBuffersPool::startingOffset,
                                                      DummyBuffersPool::aggregatedSmallBuffersPoolSize,
                                                      DummyBuffersPool::chunkAlignment,
                                                      DummyBuffersPool::smallBufferThreshold});

    auto buffersAllocator = DummyBuffersAllocator{};
    buffersAllocator.addNewBufferPool(std::move(pool1));
    buffersAllocator.addNewBufferPool(std::move(pool2));

    auto chunkSize = sizeof(DummyBuffer) / 16;
    auto chunkOffset = sizeof(DummyBuffer) / 2;
    for (size_t i = 0; i < 3; i++) {
        auto exampleOffset = chunkOffset + i * chunkSize * 2;
        buffersAllocator.tryFreeFromPoolBuffer(buffer1, exampleOffset, chunkSize);
        buffersAllocator.tryFreeFromPoolBuffer(buffer2, exampleOffset, chunkSize);
    }

    auto &chunksToFree1 = buffersAllocator.bufferPools[0].chunksToFree;
    auto &chunksToFree2 = buffersAllocator.bufferPools[1].chunksToFree;
    EXPECT_EQ(chunksToFree1.size(), 3u);
    EXPECT_EQ(chunksToFree2.size(), 3u);

    otherMemoryManager->deferAllocInUse = true;
    buffersAllocator.drain();
    EXPECT_EQ(chunksToFree1.size(), 0u);
    EXPECT_EQ(chunksToFree2.size(), 3u);
}