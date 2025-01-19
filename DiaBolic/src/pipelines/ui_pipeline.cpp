#include "pipelines/ui_pipeline.hpp"

#include "utility/shader_compiler.hpp"
#include "utility/dx12_helpers.hpp"
#include "renderer.hpp"
#include "command_queue.hpp"

using namespace Util;
using namespace DirectX;

UIPipeline::UIPipeline(Renderer& renderer) :
	_renderer(renderer)
{
	CreatePipeline();
	InitializeAssets();
}

UIPipeline::~UIPipeline()
{

}

void UIPipeline::PopulateCommandlist(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList)
{

}

void UIPipeline::Update(float deltaTime)
{

}

void UIPipeline::CreatePipeline()
{
	const auto& vertexShaderBlob = ShaderCompiler::Compile(ShaderTypes::Vertex, L"assets/shaders/dialogue_box.hlsl", L"VSmain");
	const auto& pixelShaderBlob = ShaderCompiler::Compile(ShaderTypes::Pixel, L"assets/shaders/dialogue_box.hlsl", L"PSmain");

	// Setup blend descriptions.
    constexpr D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {
        .BlendEnable = FALSE,
        .LogicOpEnable = FALSE,
        .SrcBlend = D3D12_BLEND_SRC_ALPHA,
        .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
        .BlendOp = D3D12_BLEND_OP_ADD,
        .SrcBlendAlpha = D3D12_BLEND_ONE,
        .DestBlendAlpha = D3D12_BLEND_ZERO,
        .BlendOpAlpha = D3D12_BLEND_OP_ADD,
        .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    D3D12_BLEND_DESC blendDesc = {
        .AlphaToCoverageEnable = FALSE,
        .IndependentBlendEnable = FALSE,
    };

    for(uint8_t i = 0; i < FRAME_COUNT; i++)
    {
        blendDesc.RenderTarget[i] = renderTargetBlendDesc;
    }

    // Setup depth stencil state.
    const D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
        .DepthEnable = FALSE,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D12_COMPARISON_FUNC_NONE,
        .StencilEnable = FALSE,
        .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
        .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
    };

    // Setup PSO.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {
        .pRootSignature = _renderer._bindlessRootSignature.Get(),
        .VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()),
        .PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()),
        .BlendState = blendDesc,
        .SampleMask = UINT32_MAX,
        .RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
        .DepthStencilState = depthStencilDesc,
        .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        .NumRenderTargets = FRAME_COUNT,
        .DSVFormat = DXGI_FORMAT_D32_FLOAT,
        .SampleDesc{.Count = 1u, .Quality = 0u},
        .NodeMask = 0u,
    };

    for(uint8_t i = 0; i < FRAME_COUNT; i++)
    {
        psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    ThrowIfFailed(_renderer._device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState)));
}

void UIPipeline::InitializeAssets()
{
    auto commandList = _renderer._copyCommandQueue->GetCommandList();

    std::vector<XMFLOAT2> quadVertices;
    std::vector<XMFLOAT2> quadUVs;
    std::vector<uint16_t> quadIndices;
}
