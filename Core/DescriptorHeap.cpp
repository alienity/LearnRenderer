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

#include "pch.h"
#include "DescriptorHeap.h"
#include "GraphicsCore.h"
#include "CommandListManager.h"

namespace LearnRenderer
{
    // Creates a new descriptor heap and reference the entire heap
    DescriptorHeapAllocationManager::DescriptorHeapAllocationManager(
        ID3D12Device* DeviceD3D12,
        IDescriptorAllocator& ParentAllocator,
        size_t                            ThisManagerId,
        const D3D12_DESCRIPTOR_HEAP_DESC& HeapDesc) :
        m_ParentAllocator{ ParentAllocator },
        m_DeviceD3D12{ DeviceD3D12 },
        m_ThisManagerId{ ThisManagerId },
        m_HeapDesc{ HeapDesc },
        m_DescriptorSize{ DeviceD3D12->GetDescriptorHandleIncrementSize(m_HeapDesc.Type) },
        m_NumDescriptorsInAllocation{ HeapDesc.NumDescriptors },
        m_FreeBlockManager{ HeapDesc.NumDescriptors }
    {
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pd3d12DescriptorHeap;
        ASSERT_SUCCEEDED(DeviceD3D12->CreateDescriptorHeap(&HeapDesc, MY_IID_PPV_ARGS(&pd3d12DescriptorHeap)));
        m_pd3d12DescriptorHeap = pd3d12DescriptorHeap;

        m_FirstCPUHandle = pd3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();

        if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            m_FirstGPUHandle = pd3d12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        }
    }

    DescriptorHeapAllocationManager::DescriptorHeapAllocationManager(
        ID3D12Device* DeviceD3D12,
        IDescriptorAllocator& ParentAllocator,
        size_t                 ThisManagerId,
        ID3D12DescriptorHeap* pd3d12DescriptorHeap,
        UINT32                 FirstDescriptor,
        UINT32                 NumDescriptors) :
        m_ParentAllocator{ ParentAllocator },
        m_DeviceD3D12{ DeviceD3D12 },
        m_ThisManagerId{ ThisManagerId },
        m_HeapDesc{ pd3d12DescriptorHeap->GetDesc() },
        m_DescriptorSize{ DeviceD3D12->GetDescriptorHandleIncrementSize(m_HeapDesc.Type) },
        m_NumDescriptorsInAllocation{ NumDescriptors },
        m_FreeBlockManager{ NumDescriptors },
        m_pd3d12DescriptorHeap{ pd3d12DescriptorHeap }
    {
        m_FirstCPUHandle = pd3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        m_FirstCPUHandle.ptr += m_DescriptorSize * FirstDescriptor;

        if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            m_FirstGPUHandle = pd3d12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
            m_FirstGPUHandle.ptr += m_DescriptorSize * FirstDescriptor;
        }
    }

    DescriptorHeapAllocationManager::~DescriptorHeapAllocationManager()
    {
        ASSERT(m_FreeBlockManager.GetFreeSize() == m_NumDescriptorsInAllocation, "Not all descriptors were released");
    }

    DescriptorHeapAllocation DescriptorHeapAllocationManager::Allocate(uint32_t Count)
    {
        ASSERT(Count > 0);

        std::lock_guard<std::mutex> LockGuard(m_FreeBlockManagerMutex);
        // Methods of VariableSizeAllocationsManager class are not thread safe!

        // Use variable-size GPU allocations manager to allocate the requested number of descriptors
        auto Allocation = m_FreeBlockManager.Allocate(Count, 1);
        if (!Allocation.IsValid())
            return DescriptorHeapAllocation{};

        ASSERT(Allocation.Size == Count);

        // Compute the first CPU and GPU descriptor handles in the allocation by
        // offsetting the first CPU and GPU descriptor handle in the range
        auto CPUHandle = m_FirstCPUHandle;
        CPUHandle.ptr += Allocation.UnalignedOffset * m_DescriptorSize;

        auto GPUHandle = m_FirstGPUHandle; // Will be null if the heap is not GPU-visible
        if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
            GPUHandle.ptr += Allocation.UnalignedOffset * m_DescriptorSize;

        m_MaxAllocatedSize = std::max(m_MaxAllocatedSize, m_FreeBlockManager.GetUsedSize());

        ASSERT(m_ThisManagerId < std::numeric_limits<UINT16>::max(), "ManagerID exceeds 16-bit range");
        return DescriptorHeapAllocation{ m_ParentAllocator, m_pd3d12DescriptorHeap.Get(), CPUHandle, GPUHandle, Count, static_cast<UINT16>(m_ThisManagerId) };
    }

    void DescriptorHeapAllocationManager::FreeAllocation(DescriptorHeapAllocation&& Allocation)
    {
        ASSERT(Allocation.GetAllocationManagerId() == m_ThisManagerId, "Invalid descriptor heap manager Id");

        if (Allocation.IsNull())
            return;

        std::lock_guard<std::mutex> LockGuard(m_FreeBlockManagerMutex);
        auto DescriptorOffset = (Allocation.GetCpuHandle().ptr - m_FirstCPUHandle.ptr) / m_DescriptorSize;
        // Methods of VariableSizeAllocationsManager class are not thread safe!
        m_FreeBlockManager.Free(DescriptorOffset, Allocation.GetNumHandles());

        // Clear the allocation
        Allocation.Reset();
    }



    //
    // CPUDescriptorHeap implementation
    //
    CPUDescriptorHeap::CPUDescriptorHeap(
        ID3D12Device* DeviceD3D12,
        UINT32                      NumDescriptorsInHeap,
        D3D12_DESCRIPTOR_HEAP_TYPE  Type,
        D3D12_DESCRIPTOR_HEAP_FLAGS Flags) :
        // clang-format off
        m_DeviceD3D12{ DeviceD3D12 },
        m_HeapDesc
        {
            Type,
            NumDescriptorsInHeap,
            Flags,
            1   // NodeMask
        },
        m_DescriptorSize{ DeviceD3D12->GetDescriptorHandleIncrementSize(Type) }
    {
        // Create one pool
        m_HeapPool.emplace_back(DeviceD3D12, *this, 0, m_HeapDesc);
        m_AvailableHeaps.insert(0);
    }

    CPUDescriptorHeap::~CPUDescriptorHeap()
    {
        ASSERT(m_CurrentSize == 0, "Not all allocations released");

        ASSERT(m_AvailableHeaps.size() == m_HeapPool.size(), "Not all descriptor heap pools are released");
        UINT32 TotalDescriptors = 0;
        for (auto& Heap : m_HeapPool)
        {
            ASSERT(Heap.GetNumAvailableDescriptors() == Heap.GetMaxDescriptors(), "Not all descriptors in the descriptor pool are released");
            TotalDescriptors += Heap.GetMaxDescriptors();
        }

    }

    DescriptorHeapAllocation CPUDescriptorHeap::Allocate(uint32_t Count)
    {
        std::lock_guard<std::mutex> LockGuard(m_HeapPoolMutex);
        // Note that every DescriptorHeapAllocationManager object instance is itself
        // thread-safe. Nested mutexes cannot cause a deadlock

        DescriptorHeapAllocation Allocation;
        // Go through all descriptor heap managers that have free descriptors
        auto AvailableHeapIt = m_AvailableHeaps.begin();
        while (AvailableHeapIt != m_AvailableHeaps.end())
        {
            auto NextIt = AvailableHeapIt;
            ++NextIt;
            // Try to allocate descriptor using the current descriptor heap manager
            Allocation = m_HeapPool[*AvailableHeapIt].Allocate(Count);
            // Remove the manager from the pool if it has no more available descriptors
            if (m_HeapPool[*AvailableHeapIt].GetNumAvailableDescriptors() == 0)
                m_AvailableHeaps.erase(*AvailableHeapIt);

            // Terminate the loop if descriptor was successfully allocated, otherwise
            // go to the next manager
            if (!Allocation.IsNull())
                break;
            AvailableHeapIt = NextIt;
        }

        // If there were no available descriptor heap managers or no manager was able
        // to suffice the allocation request, create a new manager
        if (Allocation.IsNull())
        {
            // Make sure the heap is large enough to accommodate the requested number of descriptors
            if (Count > m_HeapDesc.NumDescriptors)
            {
                //LOG_INFO_MESSAGE("Number of requested CPU descriptors handles (", Count, ") exceeds the descriptor heap size (", m_HeapDesc.NumDescriptors, "). Increasing the number of descriptors in the heap");
            }
            m_HeapDesc.NumDescriptors = std::max(m_HeapDesc.NumDescriptors, static_cast<UINT>(Count));
            // Create a new descriptor heap manager. Note that this constructor creates a new D3D12 descriptor
            // heap and references the entire heap. Pool index is used as manager ID
            m_HeapPool.emplace_back(m_DeviceD3D12, *this, m_HeapPool.size(), m_HeapDesc);
            auto NewHeapIt = m_AvailableHeaps.insert(m_HeapPool.size() - 1);
            ASSERT(NewHeapIt.second);

            // Use the new manager to allocate descriptor handles
            Allocation = m_HeapPool[*NewHeapIt.first].Allocate(Count);
        }

        m_CurrentSize += static_cast<UINT32>(Allocation.GetNumHandles());
        m_MaxSize = std::max(m_MaxSize, m_CurrentSize);

        return Allocation;
    }

    // Method is called from ~DescriptorHeapAllocation()
    void CPUDescriptorHeap::Free(DescriptorHeapAllocation&& Allocation)
    {
        /*
        struct StaleAllocation
        {
            DescriptorHeapAllocation Allocation;
            CPUDescriptorHeap* Heap;

            // clang-format off
            StaleAllocation(DescriptorHeapAllocation&& _Allocation, CPUDescriptorHeap& _Heap)noexcept :
                Allocation{ std::move(_Allocation) },
                Heap{ &_Heap }
            {
            }

            StaleAllocation(const StaleAllocation&) = delete;
            StaleAllocation& operator= (const StaleAllocation&) = delete;
            StaleAllocation& operator= (StaleAllocation&&) = delete;

            StaleAllocation(StaleAllocation&& rhs)noexcept :
                Allocation{ std::move(rhs.Allocation) },
                Heap{ rhs.Heap }
            {
                rhs.Heap = nullptr;
            }
            // clang-format on

            ~StaleAllocation()
            {
                if (Heap != nullptr)
                    Heap->FreeAllocation(std::move(Allocation));
            }
        };
        //m_DeviceD3D12Impl.SafeReleaseDeviceObject(StaleAllocation{ std::move(Allocation), *this }, CmdQueueMask);
        StaleAllocation{ std::move(Allocation), *this };
        */
        this->FreeAllocation(std::move(Allocation));
    }

    void CPUDescriptorHeap::FreeAllocation(DescriptorHeapAllocation&& Allocation)
    {
        std::lock_guard<std::mutex> LockGuard(m_HeapPoolMutex);
        auto                        ManagerId = Allocation.GetAllocationManagerId();
        m_CurrentSize -= static_cast<UINT32>(Allocation.GetNumHandles());
        m_HeapPool[ManagerId].FreeAllocation(std::move(Allocation));
        // Return the manager to the pool of available managers
        ASSERT(m_HeapPool[ManagerId].GetNumAvailableDescriptors() > 0);
        m_AvailableHeaps.insert(ManagerId);
    }

}