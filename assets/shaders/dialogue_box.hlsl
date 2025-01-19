#include "constant_buffers.hlsli"

struct VSOutput
{
    float2 uv : TEXCOORD;
    float4 position : SV_POSITION;
};

ConstantBuffer<DialogueBoxResources> dialogueResources : register(b0);

VSOutput VSmain(uint vertexID : SV_VertexID)
{
    StructuredBuffer<float3> positionBuffer = ResourceDescriptorHeap[dialogueResources.positionBufferIndex];
    StructuredBuffer<float3> normalBuffer = ResourceDescriptorHeap[dialogueResources.normalBufferIndex];

    VSOutput result;
    result.position = mul(dialogueResources.transform, float4(positionBuffer[vertexID], 1.0f));
    result.uv = uvBuffer[vertexID];

    return result;
}

SamplerState defaultSampler : register(s0);

float4 PSmain(VSOutput PSinput) : SV_Target0
{
    Texture2D<float4> albedoTexture = ResourceDescriptorHeap[dialogueResources.textureIndex];
    return pow(albedoTexture.Sample(defaultSampler, PSinput.uv), 1.0 / 2.2);
}