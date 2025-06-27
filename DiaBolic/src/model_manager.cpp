#include "model_manager.hpp"

#include "resources.hpp"
#include "renderer.hpp"

void ModelManager::DrawModels(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList, const std::shared_ptr<Camera> camera)
{
    for(auto& model : _loadedGPUModels)
    {
        model->Draw(commandList, camera);
    }
}

void ModelManager::LoadQueuedModelsAsync()
{
    for(auto& modelPath : _queuedModelPaths)
    {
        _futureCPUModels.push_back(std::async(std::launch::async, [&, modelPath]() { return ParseModelCPU(modelPath); }));
    }
    // TODO: clear queued model paths vector
}

void ModelManager::LoadModelsGPU(Renderer& renderer)
{
    // Wait for models to be done parsing
    for(auto& parsedFuture : _futureCPUModels)
    {
        _loadedCPUModels.push_back(parsedFuture.get());
    }
    _futureCPUModels.clear();

    // Load GPU resources
    for(auto& parsedModel : _loadedCPUModels)
    {
        _loadedGPUModels.push_back(std::make_shared<Model>(renderer, parsedModel));
    }
    _loadedCPUModels.clear();
}

ModelCPUData ModelManager::ParseModelCPU(const std::string& filePath)
{
    // TODO: port part of model loading from LoadModel to here
}