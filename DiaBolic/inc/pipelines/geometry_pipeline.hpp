#pragma once

class Renderer;
class Model;
struct Camera;

class GeometryPipeline
{
public:
	GeometryPipeline(Renderer& renderer, std::shared_ptr<Camera>& camera);
	~GeometryPipeline();

	void PopulateCommandlist(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList);
	void Update(float deltaTime);
private:
	Renderer& _renderer;
	std::shared_ptr<Camera> _camera;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState{};

	std::vector<Model> _models;

	void CreatePipeline();
	void InitializeAssets();
};