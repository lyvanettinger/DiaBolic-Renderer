#include "constant_buffers.hlsli"

struct VSOutput
{
    float2 uv : TEXCOORD;
    float3 normal : NORMAL0;
    float4 clip_position : SV_POSITION;
    float3 position : POSITION;
};

ConstantBuffer<RenderResources> renderResources : register(b0);

VSOutput VSmain(uint vertexID : SV_VertexID)
{
    StructuredBuffer<float3> positionBuffer = ResourceDescriptorHeap[renderResources.positionBufferIndex];
    StructuredBuffer<float2> uvBuffer = ResourceDescriptorHeap[renderResources.uvBufferIndex];
    StructuredBuffer<float3> normalBuffer = ResourceDescriptorHeap[renderResources.normalBufferIndex];

    VSOutput result;
    float4 w_position = mul(renderResources.MVP, float4(positionBuffer[vertexID], 1.0f));
    result.clip_position = mul(renderResources.CameraVP, w_position);
    result.position = w_position.xyz;
    result.normal = normalBuffer[vertexID]; // TODO: multiply with inverse transpose
    result.uv = uvBuffer[vertexID];

    return result;
}

SamplerState defaultSampler : register(s0);

float4 PSmain(VSOutput PSinput) : SV_Target0
{
    if(renderResources.useTexture)
    {
        Texture2D<float4> albedoTexture = ResourceDescriptorHeap[renderResources.textureIndex];
        return pow(albedoTexture.Sample(defaultSampler, PSinput.uv), 1.0 / 2.2);
    }
    return float4(1.0f, 0.0f, 1.0f, 1.0f);
}