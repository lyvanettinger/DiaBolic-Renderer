#include "resources.hpp"

#include "renderer.hpp"
#include "camera.hpp"
#include "utility/log.hpp"

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

Model::Model(const std::string& fileName)
{
    LoadModel(fileName);
}

void Model::Draw(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera)
{
    for(auto mesh : _meshes)
    {
        commandList->IASetIndexBuffer(&mesh->GetIndexBufferView());

        // Update the MVP matrix
        XMMATRIX mvpMatrix = XMMatrixMultiply(camera->model, camera->view);
        mvpMatrix = XMMatrixMultiply(mvpMatrix, camera->projection);
        mesh->SetMVP(mvpMatrix);
        commandList->SetGraphicsRoot32BitConstants(0, 64, &mesh->GetRenderResources(), 0);

        commandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
    }
}

void Model::LoadModel(const std::string& fileName)
{
    fs::path filePath("assets/models/" + fileName);
    if (!exists(filePath))
    {
        dblog::error("%s not found.", fileName.c_str());
        return;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        dblog::error("ASSIMP: %s", importer.GetErrorString());
        return;
    }
    _directory = filePath.string().substr(0, filePath.string().find_last_of('/'));

    ProcessNode(*scene, *scene->mRootNode);
}

void Model::ProcessNode(const aiScene& scene, aiNode& node)
{
    // process meshes and its materials
    for(uint32_t i = 0; i < node.mNumMeshes; ++i)
    {
        auto mesh = scene.mMeshes[node.mMeshes[i]];
        _meshes.push_back(ProcessMesh(*mesh));
        _materials.push_back(ProcessMaterial(*scene.mMaterials[mesh->mMaterialIndex]));
    }

    // continue recursive node loading process
    for(uint32_t i = 0; i < node.mNumChildren; ++i)
    {
        ProcessNode(scene, *node.mChildren[i]);
    }
}

std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh& mesh)
{
    std::vector<XMFLOAT3> positions;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> uvs;
    std::vector<uint16_t> indices;

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

        if(mesh.mTextureCoords[0])
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

    return std::make_shared<Mesh>(positions, normals, uvs, indices);
}

std::shared_ptr<Material> Model::ProcessMaterial(aiMaterial& material)
{
    return std::make_shared<Material>();
}

Mesh::Mesh(std::vector<XMFLOAT3> p, std::vector<XMFLOAT3> n, std::vector<XMFLOAT2> t, std::vector<uint16_t> i)
{
    positions = p;
    normals = n;
    uvs = t;
    indices = i;

    // TODO: setup mesh
}

// D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexBufferView()
// {
//     D3D12_INDEX_BUFFER_VIEW indexBufferView;
//     indexBufferView.BufferLocation = indexBuffer.resource->GetGPUVirtualAddress();
//     indexBufferView.Format = indexType;

//     switch(indexType)
//     {
//     case DXGI_FORMAT_R16_UINT:
//     default:
//         indexBufferView.SizeInBytes = static_cast<UINT>(indexCount * sizeof(uint16_t));
//         break;
//     }

//     return indexBufferView;
// }