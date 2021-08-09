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
//

#include "Common.hlsli"

cbuffer VSConstants : register(b0)
{
    float4x4 Proj;
    float4x4 View;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv0 : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv0 : TEXCOORD0;
};

[RootSignature(Test_RootSig)]
VSOutput main(VSInput vsInput)
{
    //// Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    //Tex = float2(uint2(VertID, VertID << 1) & 2);
    //Pos = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);
    VSOutput vsOutput;
    vsOutput.position = mul(Proj, mul(View, float4(vsInput.position, 1.0)));
    //vsOutput.position = float4(vsInput.position, 1.0);
    vsOutput.uv0 = vsInput.uv0;
    return vsOutput;
}
