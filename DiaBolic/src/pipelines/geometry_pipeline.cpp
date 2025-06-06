#include "resources.hpp"
#include "utility/resource_util.hpp"
#include "utility/dx12_helpers.hpp"

#include "pipelines/geometry_pipeline.hpp"

#include "command_queue.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "descriptor_heap.hpp"
#include "utility/shader_compiler.hpp"

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

void GeometryPipeline::PopulateCommandlist(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList)
{
    // Set necessary stuff.
    commandList->SetPipelineState(_pipelineState.Get());
    commandList->SetGraphicsRootSignature(_renderer.GetBindlessRootSignature().Get());

    // Start recording.
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for(auto& model : _models)
    {
        model.Draw(commandList, _camera);
    }
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
    const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
    _camera->model = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

    // Update the view matrix.
    _camera->view = DirectX::XMMatrixLookAtLH(_camera->position, _camera->position + _camera->front, _camera->up);

    // Update the projection matrix.
    _camera->projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(_camera->fov), _renderer.GetAspectRatio(), 0.1f, 100.0f);
}

void GeometryPipeline::CreatePipeline()
{
    const auto& vertexShaderBlob = ShaderCompiler::Compile(ShaderTypes::Vertex, L"assets/shaders/cube_spin.hlsl", L"VSmain").shaderBlob;
    const auto& pixelShaderBlob = ShaderCompiler::Compile(ShaderTypes::Pixel, L"assets/shaders/cube_spin.hlsl", L"PSmain").shaderBlob;

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
        .pRootSignature = _renderer.GetBindlessRootSignature().Get(),
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

    ThrowIfFailed(_renderer.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState)));
}

void GeometryPipeline::InitializeAssets()
{
    auto commandList = _renderer.GetCopyCommandQueue().GetCommandList();

    _models.emplace_back(Model("Sting/Sting-Sword lowpoly.fbx"));

    // std::vector<XMFLOAT3> cubeVertices;
    // std::vector<XMFLOAT3> cubeNormals;
    // std::vector<XMFLOAT2> cubeUVs;
    // std::vector<uint16_t> cubeIndices;
    // CreateCube(cubeVertices, cubeNormals, cubeUVs, cubeIndices, 2.5f);

    // // Create the positions buffer.
    // ComPtr<ID3D12Resource> positionIntermediateBuffer;
    // LoadBufferResource(_renderer.GetDevice(), commandList,
    //     &_positionBuffer.resource, &positionIntermediateBuffer,
    //     cubeVertices.size(), sizeof(XMFLOAT3), cubeVertices.data());
    // _positionBuffer.resource->SetName(L"Cube Positions");

    // const D3D12_SHADER_RESOURCE_VIEW_DESC positionDesc = {
    //     .Format = DXGI_FORMAT_UNKNOWN,
    //     .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
    //     .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    //     .Buffer = {
    //         .FirstElement = 0u,
    //         .NumElements = static_cast<UINT>(cubeVertices.size()),
    //         .StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT3)),
    //       },
    // };
    // _positionBuffer.srvIndex = _renderer.CreateSrv(positionDesc, _positionBuffer.resource);


    // // Create the normals buffer.
    // ComPtr<ID3D12Resource> normalsIntermediateBuffer;
    // LoadBufferResource(_renderer.GetDevice(), commandList,
    //     &_normalBuffer.resource, &normalsIntermediateBuffer,
    //     cubeNormals.size(), sizeof(XMFLOAT3), cubeNormals.data());
    // _normalBuffer.resource->SetName(L"Cube Normals");

    // const D3D12_SHADER_RESOURCE_VIEW_DESC normalsDesc = {
    //     .Format = DXGI_FORMAT_UNKNOWN,
    //     .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
    //     .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    //     .Buffer = {
    //         .FirstElement = 0u,
    //         .NumElements = static_cast<UINT>(cubeNormals.size()),
    //         .StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT3)),
    //       },
    // };
    // _normalBuffer.srvIndex = _renderer.CreateSrv(normalsDesc, _normalBuffer.resource);


    // // Create the uvs buffer.
    // ComPtr<ID3D12Resource> uvIntermediateBuffer;
    // LoadBufferResource(_renderer.GetDevice(), commandList,
    //     &_uvBuffer.resource, &uvIntermediateBuffer,
    //     cubeUVs.size(), sizeof(XMFLOAT2), cubeUVs.data());
    // _uvBuffer.resource->SetName(L"Cube UVs");

    // const D3D12_SHADER_RESOURCE_VIEW_DESC uvDesc = {
    //     .Format = DXGI_FORMAT_UNKNOWN,
    //     .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
    //     .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    //     .Buffer = {
    //         .FirstElement = 0u,
    //         .NumElements = static_cast<UINT>(cubeUVs.size()),
    //         .StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT2)),
    //       },
    // };
    // _uvBuffer.srvIndex = _renderer.CreateSrv(uvDesc, _uvBuffer.resource);


    // // Create the index buffer.
    // ComPtr<ID3D12Resource> indexIntermediateBuffer;
    // LoadBufferResource(_renderer.GetDevice(), commandList,
    //     &_indexBuffer, &indexIntermediateBuffer,
    //     cubeIndices.size(), sizeof(uint16_t), cubeIndices.data());
    // _indexCount = static_cast<uint32_t>(cubeIndices.size());
    // _indexBuffer->SetName(L"Cube Indices");

    // _indexBufferView = {
    //     .BufferLocation = _indexBuffer->GetGPUVirtualAddress(),
    //     .SizeInBytes = static_cast<UINT>(_indexCount * sizeof(uint16_t)),
    //     .Format = DXGI_FORMAT_R16_UINT,
    // };

    // // Create the texture.
    // ComPtr<ID3D12Resource> albedoIntermediateBuffer;
    // DXGI_FORMAT format{};
    // LoadTextureFromFile(_renderer.GetDevice(), commandList,
    //     &_albedoTexture.resource, &albedoIntermediateBuffer,
    //     L"assets/textures/Utila.jpeg", format);
    // _albedoTexture.resource->SetName(L"Utila.jpeg");

    // const D3D12_SHADER_RESOURCE_VIEW_DESC textureDesc = {
    //     .Format = format,
    //     .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
    //     .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
    //     .Texture2D = {
    //         .MostDetailedMip = 0u,
    //         .MipLevels = 1u,
    //         .PlaneSlice = 0u,
    //       },
    // };
    // _albedoTexture.srvIndex = _renderer.CreateSrv(textureDesc, _albedoTexture.resource);

    // // Set render resources.
    // _renderResources.positionBufferIndex = _positionBuffer.srvIndex;
    // _renderResources.normalBufferIndex = _normalBuffer.srvIndex;
    // _renderResources.uvBufferIndex = _uvBuffer.srvIndex;
    // _renderResources.textureIndex = _albedoTexture.srvIndex;

    // Execute list
    uint64_t fenceValue = _renderer.GetCopyCommandQueue().ExecuteCommandList(commandList);
    _renderer.GetCopyCommandQueue().WaitForFenceValue(fenceValue);
}