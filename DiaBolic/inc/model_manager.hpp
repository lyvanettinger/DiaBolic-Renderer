#pragma once

class Model;
class Node;
struct Camera;

/// CPU RESOURCES
struct MeshCPUData 
{
    std::vector<XMFLOAT3> positions;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> uvs;
    std::vector<uint16_t> indices;
    uint32_t materialIndex;
};

struct MaterialCPUData 
{
    std::vector<std::string> diffuseTexturePaths;
    std::vector<std::string> metallicRoughnessTexturePaths;
    std::vector<std::string> emissiveTexturePaths;
    std::vector<std::string> normalTexturePaths;
    std::vector<std::string> occlusionTexturePaths;
};

struct ModelCPUData 
{
    std::vector<MeshCPUData> meshes;
    std::vector<MaterialCPUData> materials;
    std::shared_ptr<Node> rootNode;
    std::string directory;
};

class ModelManager
{
public:
    void DrawModels(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera);

    void QueueModel(std::string modelPath) { _queuedModelPaths.push_back("assets/models/" + modelPath); }
    void LoadQueuedModelsAsync();
    void LoadModelsGPU(Renderer& renderer);

private:
    ModelCPUData ParseModelCPU(const std::string& filePath);

    std::vector<std::string> _queuedModelPaths;
    std::vector<std::future<ModelCPUData>> _futureCPUModels;
    std::vector<ModelCPUData> _loadedCPUModels;
    std::vector<std::shared_ptr<Model>> _loadedGPUModels;
}