#include "bindless.hlsli"
#include "constant_buffers.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float2 uv : TEXCOORD;
    float3 normal : NORMAL0;
    float4 position : SV_POSITION;
};

ConstantBuffer<UberCB> uberCB : register(b0);

[RootSignature(BindlessRootSignature)]
VSOutput VSMain(VSInput VSinput)
{
    VSOutput result;

    result.position = mul(mvp.MVP, float4(VSinput.position, 1.0f));
    result.normal = VSinput.normal;
    result.uv = VSinput.uv;

    return result;
}

// ----------------------------------------------------------------

ConstantBuffer<RenderResources> renderResources : register(b1);

[RootSignature(BindlessRootSignature)]
float4 PSMain(VSOutput PSinput) : SV_TARGET
{
    Texture2D<float4> albedoTexture = ResourceDescriptorHeap[renderResources.albedoTextureIndex];
    return albedoTexture.Sample(DefaultSampler, PSinput.uv);
}