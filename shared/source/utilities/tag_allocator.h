/*
 * Copyright (C) 2017-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/helpers/aligned_memory.h"
#include "shared/source/helpers/debug_helpers.h"
#include "shared/source/memory_manager/memory_manager.h"
#include "shared/source/utilities/idlist.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <type_traits>
#include <vector>

namespace NEO {
class GraphicsAllocation;

template <typename TagType>
class TagAllocator;

template <typename TagType>
class TagNode;

class TagAllocatorBase;

class TagNodeBase : public NonCopyableOrMovableClass {
  public:
    virtual ~TagNodeBase() = default;

    MultiGraphicsAllocation *getBaseGraphicsAllocation() const;

    uint64_t getGpuAddress() const { return gpuAddress; }

    void incRefCount() { refCount++; }

    uint32_t refCountFetchSub(uint32_t value) { return refCount.fetch_sub(value); }

    MOCKABLE_VIRTUAL void returnTag();

    virtual void initialize() = 0;

    bool canBeReleased() const;

    virtual void *getCpuBase() const = 0;

    void setDoNotReleaseNodes(bool doNotRelease) { doNotReleaseNodes = doNotRelease; }

    void setProfilingCapable(bool capable) { profilingCapable = capable; }

    bool isProfilingCapable() const { return profilingCapable; }

    void incImplicitCpuDependenciesCount() { implicitCpuDependenciesCount++; }

    uint32_t getImplicitCpuDependenciesCount() const { return implicitCpuDependenciesCount.load(); }

    const TagAllocatorBase *getAllocator() const { return allocator; }

    // TagType specific calls
    virtual bool isCompleted() const = 0;
    virtual void assignDataToAllTimestamps(uint32_t packetIndex, void *source) = 0;

    virtual size_t getGlobalStartOffset() const = 0;
    virtual size_t getContextStartOffset() const = 0;
    virtual size_t getContextEndOffset() const = 0;
    virtual size_t getGlobalEndOffset() const = 0;
    virtual size_t getImplicitGpuDependenciesCountOffset() const = 0;

    virtual uint64_t getContextStartValue(uint32_t packetIndex) const = 0;
    virtual uint64_t getGlobalStartValue(uint32_t packetIndex) const = 0;
    virtual uint64_t getContextEndValue(uint32_t packetIndex) const = 0;
    virtual uint64_t getGlobalEndValue(uint32_t packetIndex) const = 0;

    virtual uint64_t &getGlobalEndRef() const = 0;
    virtual uint64_t &getContextCompleteRef() const = 0;

    virtual void setPacketsUsed(uint32_t used) = 0;
    virtual uint32_t getPacketsUsed() const = 0;

    virtual size_t getSinglePacketSize() const = 0;

    virtual uint32_t getImplicitGpuDependenciesCount() const = 0;

    virtual MetricsLibraryApi::QueryHandle_1_0 &getQueryHandleRef() const = 0;

  protected:
    TagNodeBase() = default;

    TagAllocatorBase *allocator = nullptr;

    MultiGraphicsAllocation *gfxAllocation = nullptr;
    uint64_t gpuAddress = 0;
    std::atomic<uint32_t> refCount{0};
    std::atomic<uint32_t> implicitCpuDependenciesCount{0};
    bool doNotReleaseNodes = false;
    bool profilingCapable = true;

    template <typename TagType>
    friend class TagAllocator;
};

template <typename TagType>
class TagNode : public TagNodeBase, public IDNode<TagNode<TagType>> {
    static_assert(!std::is_polymorphic<TagType>::value,
                  "This structure is consumed by GPU and has to follow specific restrictions for padding and size");

  public:
    TagType *tagForCpuAccess;

    void initialize() override {
        tagForCpuAccess->initialize();
        implicitCpuDependenciesCount.store(0);
        setProfilingCapable(true);
    }

    void *getCpuBase() const override { return tagForCpuAccess; }

    void assignDataToAllTimestamps(uint32_t packetIndex, void *source) override;

    bool isCompleted() const override;

    size_t getGlobalStartOffset() const override;
    size_t getContextStartOffset() const override;
    size_t getContextEndOffset() const override;
    size_t getGlobalEndOffset() const override;
    size_t getImplicitGpuDependenciesCountOffset() const override;

    uint64_t getContextStartValue(uint32_t packetIndex) const override;
    uint64_t getGlobalStartValue(uint32_t packetIndex) const override;
    uint64_t getContextEndValue(uint32_t packetIndex) const override;
    uint64_t getGlobalEndValue(uint32_t packetIndex) const override;

    uint64_t &getGlobalEndRef() const override;
    uint64_t &getContextCompleteRef() const override;

    void setPacketsUsed(uint32_t used) override;
    uint32_t getPacketsUsed() const override;

    size_t getSinglePacketSize() const override;

    uint32_t getImplicitGpuDependenciesCount() const override;

    MetricsLibraryApi::QueryHandle_1_0 &getQueryHandleRef() const override;
};

class TagAllocatorBase {
  public:
    virtual ~TagAllocatorBase() { cleanUpResources(); };

    virtual void returnTag(TagNodeBase *node) = 0;

    virtual TagNodeBase *getTag() = 0;

  protected:
    TagAllocatorBase() = delete;

    TagAllocatorBase(const std::vector<uint32_t> &rootDeviceIndices, MemoryManager *memMngr, size_t tagCount,
                     size_t tagAlignment, size_t tagSize, bool doNotReleaseNodes,
                     DeviceBitfield deviceBitfield);

    virtual void returnTagToFreePool(TagNodeBase *node) = 0;

    virtual void returnTagToDeferredPool(TagNodeBase *node) = 0;

    virtual void releaseDeferredTags() = 0;

    void cleanUpResources();

    std::vector<std::unique_ptr<MultiGraphicsAllocation>> gfxAllocations;
    const DeviceBitfield deviceBitfield;
    std::vector<uint32_t> rootDeviceIndices;
    uint32_t maxRootDeviceIndex = 0;
    MemoryManager *memoryManager;
    size_t tagCount;
    size_t tagSize;
    bool doNotReleaseNodes = false;

    std::mutex allocatorMutex;
};

template <typename TagType>
class TagAllocator : public TagAllocatorBase {
  public:
    using NodeType = TagNode<TagType>;

    TagAllocator(const std::vector<uint32_t> &rootDeviceIndices, MemoryManager *memMngr, size_t tagCount,
                 size_t tagAlignment, size_t tagSize, bool doNotReleaseNodes,
                 DeviceBitfield deviceBitfield);

    TagNodeBase *getTag() override;

    void returnTag(TagNodeBase *node) override;

  protected:
    TagAllocator() = delete;

    void returnTagToFreePool(TagNodeBase *node) override;

    void returnTagToDeferredPool(TagNodeBase *node) override;

    void releaseDeferredTags() override;

    void populateFreeTags();

    IDList<NodeType> freeTags;
    IDList<NodeType> usedTags;
    IDList<NodeType> deferredTags;

    std::vector<std::unique_ptr<NodeType[]>> tagPoolMemory;
};
} // namespace NEO

#include "shared/source/utilities/tag_allocator.inl"
