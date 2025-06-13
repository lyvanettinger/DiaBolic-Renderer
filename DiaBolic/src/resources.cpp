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
        commandList->IASetIndexBuffer(&mesh->indexBufferView);

        // Update the MVP matrix
        XMMATRIX mvpMatrix = XMMatrixMultiply(camera->model, camera->view);
        mvpMatrix = XMMatrixMultiply(mvpMatrix, camera->projection);
        mesh->renderResources.MVP = mvpMatrix;
        commandList->SetGraphicsRoot32BitConstants(0, 64, &mesh->renderResources, 0);

        commandList->DrawIndexedInstanced(mesh->indexCount, 1, 0, 0, 0);
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
        _meshes.push_back(ProcessMesh(scene, *mesh));
        _materials.push_back(ProcessMaterial(*scene.mMaterials[mesh->mMaterialIndex]));
    }

    // continue recursive node loading process
    for(uint32_t i = 0; i < node.mNumChildren; ++i)
    {
        ProcessNode(scene, *node.mChildren[i]);
    }
}

std::shared_ptr<Mesh> Model::ProcessMesh(const aiScene& scene, aiMesh& mesh)
{
    return std::make_shared<Mesh>();
}

std::shared_ptr<Material> Model::ProcessMaterial(aiMaterial& material)
{
    return std::make_shared<Material>();
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