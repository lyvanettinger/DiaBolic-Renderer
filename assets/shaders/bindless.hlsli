#pragma once

#define BindlessRootSignature                                                                                          \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | "                              \
    "SAMPLER_HEAP_DIRECTLY_INDEXED),"                                                                                  \
    "RootConstants(b0, num32BitConstants=64, visibility = SHADER_VISIBILITY_ALL),"                                     \
    "StaticSampler(s0, filter = D3D12_FILTER_ANISOTROPIC, addressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, addressV = "     \
    "D3D12_TEXTURE_ADDRESS_MODE_WRAP, addressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP),"

SamplerState defaultSampler : register(s0);