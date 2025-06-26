#pragma once

#include "../../assets/shaders/constant_buffers.hlsli"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model;
struct Camera;

struct Buffer
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource{};

    uint32_t srvIndex{};
    uint32_t uavIndex{};
    uint32_t cbvIndex{};
};

class Mesh
{
public:
    Mesh(std::vector<DirectX::XMFLOAT3> positions, std::vector<DirectX::XMFLOAT3> normals, std::vector<DirectX::XMFLOAT2> uvs, std::vector<uint16_t> indices);
    ~Mesh() { };

    D3D12_INDEX_BUFFER_VIEW const& GetIndexBufferView() const { return indexBufferView; }
    RenderResources const& GetRenderResources() const { return renderResources; }
    uint32_t const& GetIndexCount() const { return indexCount; }

    void SetMVP(DirectX::XMMATRIX mvp) { renderResources.MVP = mvp; }

private:
    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT2> uvs;
    std::vector<uint16_t> indices;

    Buffer positionBuffer{};
    Buffer normalBuffer{};
    Buffer uvBuffer{};
    Buffer indexBuffer{};

    D3D12_INDEX_BUFFER_VIEW indexBufferView{};
    DXGI_FORMAT indexType{};

    uint32_t materialIndex{};
    uint32_t indexCount{};
    uint32_t vertexCount{};

    RenderResources renderResources;
};

struct Texture
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource{};

    uint32_t srvIndex{};
    uint32_t uavIndex{};

    int width = 0;
    int height = 0;
    int channels = 0;
};

struct Material
{
    DirectX::XMFLOAT4 baseColorFactor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  // 16
    bool useBaseTexture = false;                                    // 4

    DirectX::XMFLOAT3 emissiveFactor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);  // 12
    bool useEmissiveTexture = false;                         // 4

    float normalTextureScale = 0.0f;  // 4
    bool useNormalTexture = false;    // 4

    float occlusionTextureStrength = 0.0f;  // 4
    bool useOcclusionTexture = false;       // 4

    bool useMetallicRoughnessTexture = false;  // 4
    float metallicFactor = 0.0f;               // 4
    float roughnessFactor = 1.0f;              // 4

    bool isUnlit = false;  // 4
    bool recieveShadows = true;

    std::shared_ptr<Texture> baseColorTexture = nullptr;
    std::shared_ptr<Texture> emissiveTexture = nullptr;
    std::shared_ptr<Texture> normalTexture = nullptr;
    std::shared_ptr<Texture> occlusionTexture = nullptr;
    std::shared_ptr<Texture> metallicRoughnessTexture = nullptr;
};

class Model
{
public:
    Model(const std::string& fileName);

    void Draw(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera);

private:
    void LoadModel(const std::string& filePath);
    void ProcessNode(const aiScene& scene, aiNode& node);
    std::shared_ptr<Mesh> ProcessMesh(aiMesh& mesh);
    std::shared_ptr<Material> ProcessMaterial(aiMaterial& material);

    std::vector<std::shared_ptr<Mesh>> _meshes;
    std::vector<std::shared_ptr<Material>> _materials;
    // TODO: implement hierarchical scene/node structure

    std::string _directory;
};