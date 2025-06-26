#include "resources.hpp"

#include "utility/resource_util.hpp"
#include "utility/log.hpp"

#include "command_queue.hpp"
#include "renderer.hpp"
#include "camera.hpp"

#include <filesystem>

namespace fs = std::filesystem;

using namespace Util;
using namespace Microsoft::WRL;

Model::Model(Renderer& renderer, const std::string& fileName)
{
    LoadModel(renderer, fileName);
}

void Model::Draw(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera)
{
    for(auto mesh : _meshes)
    {
        commandList->IASetIndexBuffer(&mesh->GetIndexBufferView());

        // Update the MVP matrix
        XMMATRIX mvpMatrix = XMMatrixMultiply(camera->model, camera->view);
        mvpMatrix = XMMatrixMultiply(mvpMatrix, camera->projection);
        RenderResources rs;
        rs.MVP = mvpMatrix;
        rs.positionBufferIndex = mesh->GetPositionBufferSRVIndex();
        rs.normalBufferIndex = mesh->GetNormalBufferSRVIndex();
        rs.uvBufferIndex = mesh->GetUVBufferSRVIndex();
        rs.textureIndex = _textures[_materials[mesh->GetMaterialIndex()]->baseColorTextureIndex]->srvIndex;
        commandList->SetGraphicsRoot32BitConstants(0, 64, &rs, 0);

        commandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
    }
}

void Model::LoadModel(Renderer& renderer, const std::string& fileName)
{
    fs::path filePath = fs::path("assets/models/") / fileName;
    if (!fs::exists(filePath))
    {
        dblog::error("[LOAD_MODEL] File {0} not found.", fileName.c_str());
        return;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        dblog::error("[ASSIMP_IMPORTER] Error string: {0}", importer.GetErrorString());
        return;
    }
    _directory = filePath.parent_path().string() + "/";

    ProcessNode(renderer, *scene, *scene->mRootNode);
}

void Model::ProcessNode(Renderer& renderer, const aiScene& scene, aiNode& node)
{
    // process meshes and its materials
    for(uint32_t i = 0; i < node.mNumMeshes; ++i)
    {
        auto mesh = scene.mMeshes[node.mMeshes[i]];
        ProcessMesh(renderer, *mesh);
        ProcessMaterial(renderer, *scene.mMaterials[mesh->mMaterialIndex]);
    }

    // continue recursive node loading process
    for(uint32_t i = 0; i < node.mNumChildren; ++i)
    {
        ProcessNode(renderer, scene, *node.mChildren[i]);
    }
}

void Model::ProcessMesh(Renderer& renderer, aiMesh& mesh)
{
    std::vector<XMFLOAT3> positions;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> uvs;
    std::vector<uint16_t> indices;

    // log if anything is missing
    if(!mesh.HasNormals())
    {
        dblog::info("Mesh has no normals.");
    }
    if(!mesh.HasTextureCoords(0))
    {
        dblog::info("Mesh has no UVs.");
    }

    for(uint32_t i = 0; i < mesh.mNumVertices; ++i)
    {
        XMFLOAT3 position;
        position.x = mesh.mVertices[i].x;
        position.y = mesh.mVertices[i].y;
        position.z = mesh.mVertices[i].z;
        positions.push_back(position);

        if(mesh.HasNormals())
        {
            XMFLOAT3 normal;
            normal.x = mesh.mNormals[i].x;
            normal.y = mesh.mNormals[i].y;
            normal.z = mesh.mNormals[i].z;
            normals.push_back(normal);
        }

        if(mesh.HasTextureCoords(0))
        {
            XMFLOAT2 uv;
            uv.x = mesh.mTextureCoords[0][i].x;
            uv.y = mesh.mTextureCoords[0][i].y;
            uvs.push_back(uv);
        }
        else
        {
            uvs.push_back(XMFLOAT2(0.0f, 0.0f));
        }
    }

    for(uint32_t i = 0; i < mesh.mNumFaces; ++i)
    {
        aiFace face = mesh.mFaces[i];

        for(uint32_t j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
    
    _meshes.push_back(std::make_shared<Mesh>(renderer, positions, normals, uvs, indices, mesh.mMaterialIndex));
}

uint32_t Model::LoadMaterialTexture(Renderer& renderer, aiMaterial& material, aiTextureType type)
{
    // check if material has this type of texture
    if(material.GetTextureCount(type) > 0)
    {
        aiString str;
        material.GetTexture(type, 0, &str);
        // check if already loaded
        for(uint32_t i = 0; i < _textures.size(); ++i)
        {
            if(std::strcmp(_textures[i]->name.data(), str.C_Str()) == 0)
            {
                dblog::info("[LOAD_MATERIAL_TEXTURE] Texture {0} already exists.", str.C_Str());
                return i;
            }
        }
        // add to vector when not found
        _textures.push_back(std::make_shared<Texture>(renderer, _directory + str.C_Str()));
        return _textures.size() - 1;
    }
    dblog::info("[LOAD_MATERIAL_TEXTURE] Couldn't find texture for type {0}", static_cast<int>(type));
    return 0;
}

void Model::ProcessMaterial(Renderer& renderer, aiMaterial& material)
{
    aiString str = material.GetName();

    // Check if material has already beed loaded
    for(uint32_t i = 0; i < _materials.size(); ++i)
    {
        if(std::strcmp(_materials[i]->name.data(), str.C_Str()) == 0)
        {
            dblog::info("[PROCESS_MATERIAL] Material {0} already exists.", str.C_Str());
            return;
        }
    }

    // Load material when not found
    auto mat = std::make_shared<Material>();
    
    // TODO: Better material loading when implementing PBR
    mat->baseColorTextureIndex = LoadMaterialTexture(renderer, material, aiTextureType_DIFFUSE);
    mat->metallicRoughnessTextureIndex = LoadMaterialTexture(renderer, material, aiTextureType_GLTF_METALLIC_ROUGHNESS);
    mat->emissiveTextureIndex = LoadMaterialTexture(renderer, material, aiTextureType_EMISSIVE);
    mat->normalTextureIndex = LoadMaterialTexture(renderer, material, aiTextureType_NORMALS);
    mat->occlusionTextureIndex = LoadMaterialTexture(renderer, material, aiTextureType_AMBIENT_OCCLUSION);
    mat->name = str.C_Str();

    _materials.push_back(std::move(mat));
}

Mesh::Mesh(Renderer& renderer, std::vector<XMFLOAT3> positions, std::vector<XMFLOAT3> normals, std::vector<XMFLOAT2> uvs, std::vector<uint16_t> indices, unsigned int materialIndex)
{
    auto commandList = renderer.GetCopyCommandQueue().GetCommandList();

    ComPtr<ID3D12Resource> positionIntermediateBuffer;
    ComPtr<ID3D12Resource> normalsIntermediateBuffer;
    ComPtr<ID3D12Resource> uvIntermediateBuffer;
    ComPtr<ID3D12Resource> indexIntermediateBuffer;

    _vertexCount = static_cast<uint32_t>(positions.size());
    _indexCount = static_cast<uint32_t>(indices.size());
    _materialIndex = materialIndex;

    // Initialize buffers
    if(!positions.empty())
    {
        LoadBufferResource(renderer.GetDevice(), commandList,
            &_positionBuffer.resource, &positionIntermediateBuffer,
            _vertexCount, sizeof(XMFLOAT3), positions.data());
        _positionBuffer.resource->SetName(L"Positions");

        const D3D12_SHADER_RESOURCE_VIEW_DESC positionDesc = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = {
                .FirstElement = 0u,
                .NumElements = static_cast<UINT>(_vertexCount),
                .StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT3)),
            },
        };
        _positionBuffer.srvIndex = renderer.CreateSrv(positionDesc, _positionBuffer.resource);
    }

    if(!normals.empty())
    {
        LoadBufferResource(renderer.GetDevice(), commandList,
            &_normalBuffer.resource, &normalsIntermediateBuffer,
            _vertexCount, sizeof(XMFLOAT3), normals.data());
        _normalBuffer.resource->SetName(L"Normals");

        const D3D12_SHADER_RESOURCE_VIEW_DESC normalsDesc = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = {
                .FirstElement = 0u,
                .NumElements = static_cast<UINT>(_vertexCount),
                .StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT3)),
            },
        };
        _normalBuffer.srvIndex = renderer.CreateSrv(normalsDesc, _normalBuffer.resource);
    }

    if(!uvs.empty())
    {
        LoadBufferResource(renderer.GetDevice(), commandList,
            &_uvBuffer.resource, &uvIntermediateBuffer,
            _vertexCount, sizeof(XMFLOAT2), uvs.data());
        _uvBuffer.resource->SetName(L"UVs");

        const D3D12_SHADER_RESOURCE_VIEW_DESC uvDesc = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = {
                .FirstElement = 0u,
                .NumElements = static_cast<UINT>(_vertexCount),
                .StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT2)),
            },
        };
        _uvBuffer.srvIndex = renderer.CreateSrv(uvDesc, _uvBuffer.resource);
    }

    if(!indices.empty())
    {
        LoadBufferResource(renderer.GetDevice(), commandList,
            &_indexBuffer.resource, &indexIntermediateBuffer,
            _indexCount, sizeof(uint16_t), indices.data());
        _indexBuffer.resource->SetName(L"Indices");

        _indexBufferView = {
            .BufferLocation = _indexBuffer.resource->GetGPUVirtualAddress(),
            .SizeInBytes = static_cast<UINT>(_indexCount * sizeof(uint16_t)),
            .Format = DXGI_FORMAT_R16_UINT,
        };
    }

    // Execute list
    uint64_t fenceValue = renderer.GetCopyCommandQueue().ExecuteCommandList(commandList);
    renderer.GetCopyCommandQueue().WaitForFenceValue(fenceValue);
}

Texture::Texture(Renderer& renderer, std::string path)
{
    auto commandList = renderer.GetCopyCommandQueue().GetCommandList();

    std::string fileName = std::filesystem::path(path).filename().string();

    ComPtr<ID3D12Resource> intermediateBuffer;
    DXGI_FORMAT format{};
    LoadTextureFromFile(renderer.GetDevice(), commandList,
        &resource, &intermediateBuffer,
        Util::StringTowString(path), format);
    resource->SetName(Util::StringTowString(fileName).c_str());
    name = fileName;

    const D3D12_SHADER_RESOURCE_VIEW_DESC textureDesc = {
        .Format = format,
        .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .Texture2D = {
            .MostDetailedMip = 0u,
            .MipLevels = 1u,
            .PlaneSlice = 0u,
          },
    };
    srvIndex = renderer.CreateSrv(textureDesc, resource);

    // Execute list
    uint64_t fenceValue = renderer.GetCopyCommandQueue().ExecuteCommandList(commandList);
    renderer.GetCopyCommandQueue().WaitForFenceValue(fenceValue);
}

Texture::Texture(Renderer& renderer, aiTexture textureData)
{
    
}