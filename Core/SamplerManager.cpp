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
// Author(s):  James Stanard
//             Alex Nankervis
//

#include "pch.h"
#include "SamplerManager.h"
#include "GraphicsCore.h"
#include "Hash.h"
#include "DescriptorHeap.h"
#include <map>

using namespace std;
using namespace Graphics;

LearnRenderer::DescriptorHeapAllocation SamplerDesc::CreateDescriptor()
{
    LearnRenderer::DescriptorHeapAllocation Handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    g_Device->CreateSampler(this, Handle.GetCpuHandle());
    return Handle;
}

void SamplerDesc::CreateDescriptor( D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    ASSERT(Handle.ptr != 0 && Handle.ptr != -1);
    g_Device->CreateSampler(this, Handle);
}
