#pragma once

#ifdef __cplusplus

#define float4 DirectX::XMFLOAT4
#define float3 DirectX::XMFLOAT3
#define float2 DirectX::XMFLOAT2

#define uint uint32_t

#define float4x4 DirectX::XMMATRIX

#define ConstantBufferStruct struct alignas(256)

#else // if HLSL

#define ConstantBufferStruct struct

#endif

ConstantBufferStruct RenderResources
{
    float4x4 MVP;
    float4x4 CameraVP;
    uint positionBufferIndex;
    uint normalBufferIndex;
    uint uvBufferIndex;
    uint textureIndex;
};