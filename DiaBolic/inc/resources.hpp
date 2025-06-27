#pragma once

#include "../../assets/shaders/constant_buffers.hlsli"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Renderer;
class Model;
struct Camera;
struct ModelCPUData;
struct MaterialCPUData;
struct MeshCPUData;

/// GPU RESOURCES
struct Buffer
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource = NULL;

    uint32_t srvIndex = 0;
    uint32_t uavIndex = 0;
    uint32_t cbvIndex = 0;
};

class Mesh
{
public:
    Mesh(Renderer& renderer, std::vector<DirectX::XMFLOAT3> positions, std::vector<DirectX::XMFLOAT3> normals, std::vector<DirectX::XMFLOAT2> uvs, std::vector<uint16_t> indices, unsigned int materialIndex);
    ~Mesh() { }

    uint32_t const& GetPositionBufferSRVIndex() { return _positionBuffer.srvIndex; }
    uint32_t const& GetNormalBufferSRVIndex() { return _normalBuffer.srvIndex; }
    uint32_t const& GetUVBufferSRVIndex() { return _uvBuffer.srvIndex; }

    D3D12_INDEX_BUFFER_VIEW const& GetIndexBufferView() const { return _indexBufferView; }
    uint32_t const& GetIndexCount() const { return _indexCount; }
    uint32_t const& GetMaterialIndex() const { return _materialIndex; }

private:
    Buffer _positionBuffer;
    Buffer _normalBuffer;
    Buffer _uvBuffer;
    Buffer _indexBuffer;

    D3D12_INDEX_BUFFER_VIEW _indexBufferView{};
    DXGI_FORMAT _indexType = DXGI_FORMAT_R16_UINT;

    uint32_t _materialIndex = 0;
    uint32_t _indexCount = 0;
    uint32_t _vertexCount = 0;
};

struct Texture
{
    Texture(Renderer& renderer, std::string path);
    Texture(Renderer& renderer, aiTexture textureData);
    ~Texture() { }

    Microsoft::WRL::ComPtr<ID3D12Resource> resource = NULL;

    uint32_t srvIndex = 0;
    uint32_t uavIndex = 0;

    std::string name = "";

    // TODO: get this somewhere.. or not
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

class Node
{
public:
    void Draw(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera);

    void SetMesh(std::shared_ptr<Mesh>& mesh) { _mesh = mesh; };
    void SetMaterial(std::shared_ptr<Material>& material) { _material = material; }
    void SetParent(std::shared_ptr<Node>& node) { _parent = node; }
    void SetTransform(DirectX::XMMATRIX transform) { _transform = transform; }
    void AddChild(std::shared_ptr<Node>& node) { _children.emplace_back(node); }

    std::shared_ptr<Mesh> const& GetMesh() const { return _mesh; }
    std::shared_ptr<Material> const& GetMaterial() const { return _material; }
    DirectX::XMMATRIX const& GetTransform() const 
    { 
        if(_parent)
        {
            return _parent->GetTransform() * _transform;
        }
        return _transform; 
    }

private:
    std::shared_ptr<Mesh> _mesh = nullptr;
    std::shared_ptr<Material> _material = nullptr;

    // TODO: Handle transforms differently?
    DirectX::XMMATRIX _transform = DirectX::XMMatrixIdentity();

    std::shared_ptr<Node> _parent = nullptr;
    std::vector<std::shared_ptr<Node>> _children;
};

class Model
{
public:
    Model(Renderer& renderer, ModelCPUData& data);

    void Draw(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera);

private:
    void LoadModel(Renderer& renderer, const std::string& filePath);
    void ProcessNode(Renderer& renderer, const aiScene& scene, aiNode& node, std::shared_ptr<Node> parentNode);
    void ProcessMesh(Renderer& renderer, aiMesh& mesh);
    void ProcessMaterial(Renderer& renderer, aiMaterial& material);
    std::shared_ptr<Texture> LoadMaterialTexture(Renderer& renderer, aiMaterial& material, aiTextureType type);

    std::vector<std::shared_ptr<Mesh>> _meshes;
    std::vector<std::shared_ptr<Material>> _materials;
    std::vector<std::shared_ptr<Texture>> _texturesLoaded; // TODO: move this to separate resource manager. this just exists to keep track of loaded textures
    std::shared_ptr<Node> _rootNode;

    std::string _directory = "";
};