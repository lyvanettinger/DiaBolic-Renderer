#include "resource_util.hpp"
#include "dx12_helpers.hpp"

#include "pipelines/geometry_pipeline.hpp"

#include "command_queue.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "descriptor_heap.hpp"
#include "shader_compiler.hpp"

using namespace Util;
using namespace Microsoft::WRL;

GeometryPipeline::GeometryPipeline(Renderer& renderer, std::shared_ptr<Camera>& camera)
    : _renderer(renderer)
    , _camera(camera)
{
    CreatePipeline();
    InitializeAssets();
}

GeometryPipeline::~GeometryPipeline()
{

}

void GeometryPipeline::PopulateCommandlist(const ComPtr<ID3D12GraphicsCommandList2>& commandList)
{
    // Set necessary stuff.
    commandList->SetPipelineState(_pipelineState.Get());
    commandList->SetGraphicsRootSignature(_rootSignature.Get());

    // Start recording.
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    commandList->IASetIndexBuffer(&_indexBufferView);

    // Update the MVP matrix.
    XMMATRIX mvpMatrix = XMMatrixMultiply(_camera->model, _camera->view);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, _camera->projection);

    // Set Root32BitConstants.
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);
    commandList->SetGraphicsRoot32BitConstants(1, sizeof(_renderResources) / 4, &_renderResources, 0);

    commandList->DrawIndexedInstanced(_indexCount, 1, 0, 0, 0);
}

void GeometryPipeline::Update(float deltaTime)
{
    static double totalTime = 0.0f;
    totalTime += deltaTime;
    if (totalTime > 4.0f)
    {
        totalTime = 0.0f;
    }

    // Update the model matrix.
    float angle = static_cast<float>(totalTime * 90.0);
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    _camera->model = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    // Update the view matrix.
    _camera->view = XMMatrixLookAtLH(_camera->position, _camera->position + _camera->front, _camera->up);

    // Update the projection matrix.
    _camera->projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(_camera->fov), _renderer._aspectRatio, 0.1f, 100.0f);
}

void GeometryPipeline::CreatePipeline()
{
    // Compile shader(s) and extract root signature
    const auto& vertexShader = ShaderCompiler::Compile(ShaderTypes::Vertex, L"assets/shaders/uber.hlsl", L"VSMain", true);
    const auto& pixelShaderBlob = ShaderCompiler::Compile(ShaderTypes::Pixel, L"assets/shaders/uber.hlsl", L"PSMain").shaderBlob;
    ThrowIfFailed(_renderer._device->CreateRootSignature(0, vertexShader.rootSignatureBlob->GetBufferPointer(), vertexShader.rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

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
        .DepthEnable = TRUE,
        .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
        .StencilEnable = FALSE,
        .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
        .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
    };

    // Setup PSO.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {
        .pRootSignature = _rootSignature.Get(),
        .VS = CD3DX12_SHADER_BYTECODE(vertexShader.shaderBlob->GetBufferPointer(), vertexShader.shaderBlob->GetBufferSize()),
        .PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()),
        .BlendState = blendDesc,
        .SampleMask = UINT32_MAX,
        .RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
        .DepthStencilState = depthStencilDesc,
        .InputLayout = { inputElementDescs, _countof(inputElementDescs) },
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

void GeometryPipeline::InitializeAssets()
{
    auto commandList = _renderer._copyCommandQueue->GetCommandList();

    std::vector<Vertex> cubeVertices;
    std::vector<uint16_t> cubeIndices;
    CreateCube(cubeVertices, cubeIndices, 1.0f);
    
    // Create the vertex buffer.
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    LoadBufferResource(_renderer._device, commandList,
        &_vertexBuffer, &intermediateVertexBuffer,
        cubeVertices.size(), sizeof(Vertex), cubeVertices.data());

    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = sizeof(Vertex) * cubeVertices.size();

    // Create the index buffer.
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
    LoadBufferResource(_renderer._device, commandList,
        &_IndexBuffer, &intermediateIndexBuffer,
        cubeIndices.size(), sizeof(uint16_t), cubeIndices.data());
    _indexCount = static_cast<int>(cubeIndices.size());

    _indexBufferView.BufferLocation = _IndexBuffer->GetGPUVirtualAddress();
    _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    _indexBufferView.SizeInBytes = _indexCount * sizeof(uint16_t);

    // Create the texture.
    ComPtr<ID3D12Resource> intermediateAlbedoBuffer;
    LoadTextureFromFile(_renderer._device, commandList,
        &_albedoTexture, &intermediateAlbedoBuffer,
        L"assets/textures/Utila.jpeg");
    _albedoTextureView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    _albedoTextureView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    _albedoTextureView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    _albedoTextureView.Texture2D.PlaneSlice = 0;
    _albedoTextureView.Texture2D.MipLevels = 1;
    _albedoTextureView.Texture2D.MostDetailedMip = 0;

    // Create the texture SRV
    _renderResources.albedoTextureIndex = _renderer._srvHeap->getCurrentDescriptorIndex();
    _albedoTextureHandle = std::make_unique<DescriptorHandle>(_renderer._srvHeap->getCurrentDescriptorHandle());
    _renderer._device->CreateShaderResourceView(_albedoTexture.Get(), &_albedoTextureView, _albedoTextureHandle->cpuDescriptorHandle);
    _renderer._srvHeap->offsetCurrentHandle();

    // Execute list
    uint64_t fenceValue = _renderer._copyCommandQueue->ExecuteCommandList(commandList);
    _renderer._copyCommandQueue->WaitForFenceValue(fenceValue);
}