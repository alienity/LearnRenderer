//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include <mutex>
#include <vector>
#include <queue>
#include <string>
#include <unordered_set>

#include "VariableSizeAllocationsManager.hpp"

namespace LearnRenderer 
{

    class DescriptorHeapAllocation;
    class DescriptorHeapAllocationManager;
    
    class IDescriptorAllocator
    {
    public:
        // Allocate Count descriptors
        virtual DescriptorHeapAllocation Allocate(uint32_t Count) = 0;
        virtual void                     Free(DescriptorHeapAllocation&& Allocation) = 0;
        virtual UINT32                   GetDescriptorSize() const = 0;
    };

    // The class represents descriptor heap allocation (continuous descriptor range in a descriptor heap)
    //
    //                  m_FirstCpuHandle
    //                   |
    //  | ~  ~  ~  ~  ~  X  X  X  X  X  X  X  ~  ~  ~  ~  ~  ~ |  D3D12 Descriptor Heap
    //                   |
    //                  m_FirstGpuHandle
    //
    class DescriptorHeapAllocation
    {
    public:
        // Creates null allocation
        DescriptorHeapAllocation() noexcept :
            // clang-format off
            m_NumHandles{ 1 }, // One null descriptor handle
            m_pDescriptorHeap{ nullptr },
            m_DescriptorSize{ 0 }
            // clang-format on
        {
            m_FirstCpuHandle.ptr = 0;
            m_FirstGpuHandle.ptr = 0;
        }

        // Initializes non-null allocation
        DescriptorHeapAllocation(IDescriptorAllocator& Allocator,
            ID3D12DescriptorHeap* pHeap,
            D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle,
            UINT32                      NHandles,
            UINT16                      AllocationManagerId) noexcept :
            // clang-format off
            m_FirstCpuHandle{ CpuHandle },
            m_FirstGpuHandle{ GpuHandle },
            m_pAllocator{ &Allocator },
            m_NumHandles{ NHandles },
            m_pDescriptorHeap{ pHeap },
            m_AllocationManagerId{ AllocationManagerId }
            // clang-format on
        {
            ASSERT(m_pAllocator != nullptr && m_pDescriptorHeap != nullptr);
            auto DescriptorSize = m_pAllocator->GetDescriptorSize();
            ASSERT(DescriptorSize < std::numeric_limits<UINT16>::max(), "DescriptorSize exceeds allowed limit");
            m_DescriptorSize = static_cast<UINT16>(DescriptorSize);
        }

        // Move constructor (copy is not allowed)
        DescriptorHeapAllocation(DescriptorHeapAllocation&& Allocation) noexcept :
            // clang-format off
            m_FirstCpuHandle{ std::move(Allocation.m_FirstCpuHandle) },
            m_FirstGpuHandle{ std::move(Allocation.m_FirstGpuHandle) },
            m_NumHandles{ std::move(Allocation.m_NumHandles) },
            m_pAllocator{ std::move(Allocation.m_pAllocator) },
            m_AllocationManagerId{ std::move(Allocation.m_AllocationManagerId) },
            m_pDescriptorHeap{ std::move(Allocation.m_pDescriptorHeap) },
            m_DescriptorSize{ std::move(Allocation.m_DescriptorSize) }
            // clang-format on
        {
            Allocation.Reset();
        }

        // Move assignment (assignment is not allowed)
        DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& Allocation) noexcept
        {
            m_FirstCpuHandle = std::move(Allocation.m_FirstCpuHandle);
            m_FirstGpuHandle = std::move(Allocation.m_FirstGpuHandle);
            m_NumHandles = std::move(Allocation.m_NumHandles);
            m_pAllocator = std::move(Allocation.m_pAllocator);
            m_AllocationManagerId = std::move(Allocation.m_AllocationManagerId);
            m_pDescriptorHeap = std::move(Allocation.m_pDescriptorHeap);
            m_DescriptorSize = std::move(Allocation.m_DescriptorSize);

            Allocation.Reset();

            return *this;
        }

        void Reset()
        {
            m_FirstCpuHandle.ptr = 0;
            m_FirstGpuHandle.ptr = 0;
            m_pAllocator = nullptr;
            m_pDescriptorHeap = nullptr;
            m_NumHandles = 0;
            m_AllocationManagerId = InvalidAllocationMgrId;
            m_DescriptorSize = 0;
        }

        // clang-format off
        DescriptorHeapAllocation(const DescriptorHeapAllocation&) = delete;
        DescriptorHeapAllocation& operator=(const DescriptorHeapAllocation&) = delete;
        // clang-format on


        // Destructor automatically releases this allocation through the allocator
        ~DescriptorHeapAllocation()
        {
            if (!IsNull() && m_pAllocator)
                m_pAllocator->Free(std::move(*this));
            // Allocation must have been disposed by the allocator
            ASSERT(IsNull(), "Non-null descriptor is being destroyed");
        }

        // Returns CPU descriptor handle at the specified offset
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT32 Offset = 0) const
        {
            ASSERT(Offset >= 0 && Offset < m_NumHandles);

            D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_FirstCpuHandle;
            CPUHandle.ptr += m_DescriptorSize * Offset;

            return CPUHandle;
        }

        // Returns GPU descriptor handle at the specified offset
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT32 Offset = 0) const
        {
            ASSERT(Offset >= 0 && Offset < m_NumHandles);
            D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle = m_FirstGpuHandle;
            GPUHandle.ptr += m_DescriptorSize * Offset;

            return GPUHandle;
        }

        template <typename HandleType>
        HandleType GetHandle(UINT32 Offset = 0) const;

        template <>
        D3D12_CPU_DESCRIPTOR_HANDLE GetHandle<D3D12_CPU_DESCRIPTOR_HANDLE>(UINT32 Offset) const
        {
            return GetCpuHandle(Offset);
        }

        template <>
        D3D12_GPU_DESCRIPTOR_HANDLE GetHandle<D3D12_GPU_DESCRIPTOR_HANDLE>(UINT32 Offset) const
        {
            return GetGpuHandle(Offset);
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const 
        {
            if (this->GetNumHandles() != 1 || this->IsNull())
                return nullptr;
            return &m_FirstCpuHandle;
        }

        operator D3D12_CPU_DESCRIPTOR_HANDLE() const 
        {
            if (this->GetNumHandles() != 1 || this->IsNull())
                printf("Handle Number is not 1\n");
            return m_FirstCpuHandle;
        }
        operator D3D12_GPU_DESCRIPTOR_HANDLE() const 
        {
            if (this->GetNumHandles() != 1 || this->IsNull() || !this->IsShaderVisible())
                printf("Is not ShaderVisible or Handle Number is not 1\n");
            return m_FirstGpuHandle;
        }


        // Returns pointer to D3D12 descriptor heap that contains this allocation
        ID3D12DescriptorHeap* GetDescriptorHeap() const { return m_pDescriptorHeap; }


        // clang-format off
        size_t GetNumHandles()          const { return m_NumHandles; }
        bool   IsNull()                 const { return m_FirstCpuHandle.ptr == 0; }
        bool   IsShaderVisible()        const { return m_FirstGpuHandle.ptr != 0; }
        size_t GetAllocationManagerId() const { return m_AllocationManagerId; }
        UINT   GetDescriptorSize()      const { return m_DescriptorSize; }
        // clang-format on

    private:
        // First CPU descriptor handle in this allocation
        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCpuHandle = { 0 };

        // First GPU descriptor handle in this allocation
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGpuHandle = { 0 };

        // Keep strong reference to the parent heap to make sure it is alive while allocation is alive - TOO EXPENSIVE
        //RefCntAutoPtr<IDescriptorAllocator> m_pAllocator;

        // Pointer to the descriptor heap allocator that created this allocation
        IDescriptorAllocator* m_pAllocator = nullptr;

        // Pointer to the D3D12 descriptor heap that contains descriptors in this allocation
        ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;

        // Number of descriptors in the allocation
        UINT32 m_NumHandles = 0;

        static constexpr UINT16 InvalidAllocationMgrId = 0xFFFF;

        // Allocation manager ID. One allocator may support several
        // allocation managers. This field is required to identify
        // the manager within the allocator that was used to create
        // this allocation
        UINT16 m_AllocationManagerId = InvalidAllocationMgrId;

        // Descriptor size
        UINT16 m_DescriptorSize = 0;
    };

    // The class performs suballocations within one D3D12 descriptor heap.
    // It uses VariableSizeAllocationsManager to manage free space in the heap
    //
    // |  X  X  X  X  O  O  O  X  X  O  O  X  O  O  O  O  |  D3D12 descriptor heap
    //
    //  X - used descriptor
    //  O - available descriptor
    //
    class DescriptorHeapAllocationManager
    {
    public:
        // Creates a new D3D12 descriptor heap
        DescriptorHeapAllocationManager(
            ID3D12Device& DeviceD3D12,
            IDescriptorAllocator& ParentAllocator,
            size_t                            ThisManagerId,
            const D3D12_DESCRIPTOR_HEAP_DESC& HeapDesc);

        // Uses subrange of descriptors in the existing D3D12 descriptor heap
        // that starts at offset FirstDescriptor and uses NumDescriptors descriptors
        DescriptorHeapAllocationManager(
            ID3D12Device& DeviceD3D12,
            IDescriptorAllocator& ParentAllocator,
            size_t                 ThisManagerId,
            ID3D12DescriptorHeap* pd3d12DescriptorHeap,
            UINT32                 FirstDescriptor,
            UINT32                 NumDescriptors);


        // = default causes compiler error when instantiating std::vector::emplace_back() in Visual Studio 2015 (Version 14.0.23107.0 D14REL)
        DescriptorHeapAllocationManager(DescriptorHeapAllocationManager&& rhs) noexcept :
            // clang-format off
            m_ParentAllocator{ rhs.m_ParentAllocator },
            m_DeviceD3D12{ rhs.m_DeviceD3D12 },
            m_ThisManagerId{ rhs.m_ThisManagerId },
            m_HeapDesc{ rhs.m_HeapDesc },
            m_DescriptorSize{ rhs.m_DescriptorSize },
            m_NumDescriptorsInAllocation{ rhs.m_NumDescriptorsInAllocation },
            m_FirstCPUHandle{ rhs.m_FirstCPUHandle },
            m_FirstGPUHandle{ rhs.m_FirstGPUHandle },
            m_MaxAllocatedSize{ rhs.m_MaxAllocatedSize },
            // Mutex is not movable
            //m_FreeBlockManagerMutex     (std::move(rhs.m_FreeBlockManagerMutex))
            m_FreeBlockManager{ std::move(rhs.m_FreeBlockManager) },
            m_pd3d12DescriptorHeap{ std::move(rhs.m_pd3d12DescriptorHeap) }
            // clang-format on
        {
            rhs.m_NumDescriptorsInAllocation = 0; // Must be set to zero so that debug check in dtor passes
            rhs.m_ThisManagerId = static_cast<size_t>(-1);
            rhs.m_FirstCPUHandle.ptr = 0;
            rhs.m_FirstGPUHandle.ptr = 0;
            rhs.m_MaxAllocatedSize = 0;
        }

        // clang-format off
        // No copies or move-assignments
        DescriptorHeapAllocationManager& operator = (DescriptorHeapAllocationManager&&) = delete;
        DescriptorHeapAllocationManager(const DescriptorHeapAllocationManager&) = delete;
        DescriptorHeapAllocationManager& operator = (const DescriptorHeapAllocationManager&) = delete;
        // clang-format on

        ~DescriptorHeapAllocationManager();

        // Allocates Count descriptors
        DescriptorHeapAllocation Allocate(uint32_t Count);
        void                     FreeAllocation(DescriptorHeapAllocation&& Allocation);

        // clang-format off
        size_t GetNumAvailableDescriptors()const { return m_FreeBlockManager.GetFreeSize(); }
        UINT32 GetMaxDescriptors()         const { return m_NumDescriptorsInAllocation; }
        size_t GetMaxAllocatedSize()       const { return m_MaxAllocatedSize; }
        // clang-format on

    private:
        IDescriptorAllocator& m_ParentAllocator;
        ID3D12Device&         m_DeviceD3D12;

        // External ID assigned to this descriptor allocations manager
        size_t m_ThisManagerId = static_cast<size_t>(-1);

        // Heap description
        const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

        const UINT m_DescriptorSize = 0;

        // Number of descriptors in the allocation.
        // If this manager was initialized as a subrange in the existing heap,
        // this value may be different from m_HeapDesc.NumDescriptors
        UINT32 m_NumDescriptorsInAllocation = 0;

        // Allocations manager used to handle descriptor allocations within the heap
        std::mutex                     m_FreeBlockManagerMutex;
        VariableSizeAllocationsManager m_FreeBlockManager;

        // Strong reference to D3D12 descriptor heap object
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pd3d12DescriptorHeap;

        // First CPU descriptor handle in the available descriptor range
        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = { 0 };

        // First GPU descriptor handle in the available descriptor range
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = { 0 };

        size_t m_MaxAllocatedSize = 0;

        // Note: when adding new members, do not forget to update move ctor
    };

    // CPU descriptor heap is intended to provide storage for resource view descriptor handles.
    // It contains a pool of DescriptorHeapAllocationManager object instances, where every instance manages
    // its own CPU-only D3D12 descriptor heap:
    //
    //           m_HeapPool[0]                m_HeapPool[1]                 m_HeapPool[2]
    //   |  X  X  X  X  X  X  X  X |, |  X  X  X  O  O  X  X  O  |, |  X  O  O  O  O  O  O  O  |
    //
    //    X - used descriptor                m_AvailableHeaps = {1,2}
    //    O - available descriptor
    //
    // Allocation routine goes through the list of managers that have available descriptors and tries to process
    // the request using every manager. If there are no available managers or no manager was able to handle the request,
    // the function creates a new descriptor heap manager and lets it handle the request
    //
    // Render device contains four CPUDescriptorHeap object instances (one for each D3D12 heap type). The heaps are accessed
    // when a texture or a buffer view is created.
    //
    class CPUDescriptorHeap final : public IDescriptorAllocator
    {
    public:
        // Initializes the heap
        CPUDescriptorHeap(
            ID3D12Device&               DeviceD3D12,
            UINT32                      NumDescriptorsInHeap,
            D3D12_DESCRIPTOR_HEAP_TYPE  Type,
            D3D12_DESCRIPTOR_HEAP_FLAGS Flags);

        // clang-format off
        CPUDescriptorHeap(const CPUDescriptorHeap&) = delete;
        CPUDescriptorHeap(CPUDescriptorHeap&&) = delete;
        CPUDescriptorHeap& operator = (const CPUDescriptorHeap&) = delete;
        CPUDescriptorHeap& operator = (CPUDescriptorHeap&&) = delete;
        // clang-format on

        ~CPUDescriptorHeap();

        virtual DescriptorHeapAllocation Allocate(uint32_t Count) override final;
        virtual void                     Free(DescriptorHeapAllocation&& Allocation) override final;
        virtual UINT32                   GetDescriptorSize() const override final { return m_DescriptorSize; }

    private:
        void FreeAllocation(DescriptorHeapAllocation&& Allocation);

        ID3D12Device& m_DeviceD3D12;

        // Pool of descriptor heap managers
        std::mutex                                   m_HeapPoolMutex;
        std::vector<DescriptorHeapAllocationManager> m_HeapPool;
        // Indices of available descriptor heap managers
        std::unordered_set<size_t>                   m_AvailableHeaps;

        D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
        const UINT                 m_DescriptorSize = 0;

        // Maximum heap size during the application lifetime - for statistic purposes
        UINT32 m_MaxSize = 0;
        UINT32 m_CurrentSize = 0;
    };

}