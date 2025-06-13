#pragma once

class Renderer;
class Model;
struct Camera;
struct Buffer;
struct Texture;

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

	// temporarily stored here (cube stuff)
	Buffer _positionBuffer{};
	Buffer _normalBuffer{};
	Buffer _uvBuffer{};
	Microsoft::WRL::ComPtr<ID3D12Resource> _indexBuffer{};
	D3D12_INDEX_BUFFER_VIEW _indexBufferView{};
	uint32_t _indexCount{};
	RenderResources _renderResources{};
	Texture _albedoTexture{};

	std::vector<Model> _models;

	void CreatePipeline();
	void InitializeAssets();
};