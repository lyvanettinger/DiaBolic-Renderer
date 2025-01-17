#pragma once

class Renderer;
struct Camera;
struct DescriptorHandle;
struct Shader;

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

	struct RenderResources
	{
		uint32_t albedoTextureIndex;
	} _renderResources;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

	// Temporarily just store these here. Usually these should be part of a model resource
	Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> _IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
	int _indexCount;
	Microsoft::WRL::ComPtr<ID3D12Resource> _albedoTexture;
	std::unique_ptr<DescriptorHandle> _albedoTextureHandle;
	D3D12_SHADER_RESOURCE_VIEW_DESC _albedoTextureView;

	void CreatePipeline();
	void InitializeAssets();
};