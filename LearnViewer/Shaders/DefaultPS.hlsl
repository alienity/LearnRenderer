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
// Author(s):	James Stanard
//              Justin Saunders (ATG)

#include "Common.hlsli"

Texture2D ColorTex : register(t0);
SamplerState BilinearSampler : register(s0);

[RootSignature(Test_RootSig)]
float4 main(float4 position : SV_Position) : SV_Target0
{
    return float4(1,0,0,1);
    //return ColorTex[(int2)position.xy];
}
