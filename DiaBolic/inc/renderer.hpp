#pragma once

class Application;
class GeometryPipeline;
class UIPipeline;
class CommandQueue;
class DescriptorHeap;
struct Camera;

class Renderer
{
public:
	Renderer(std::shared_ptr<Application> app);
	~Renderer();
    
    void Update(float deltaTime);
	void Render();

    // Getters
    CommandQueue& GetCopyCommandQueue() { return *_copyCommandQueue; }
    Microsoft::WRL::ComPtr<ID3D12Device2>& GetDevice() { return _device; }
    Microsoft::WRL::ComPtr<ID3D12RootSignature>& GetBindlessRootSignature() { return _bindlessRootSignature; }
    float GetAspectRatio() { return _aspectRatio; }

    [[nodiscard]] uint32_t CreateCbv(const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvCreationDesc) const;
	[[nodiscard]] uint32_t CreateSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC& srvCreationDesc, const Microsoft::WRL::ComPtr<ID3D12Resource>& resource) const;
	[[nodiscard]] uint32_t CreateUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC& uavCreationDesc, const Microsoft::WRL::ComPtr<ID3D12Resource>& resource) const;
	[[nodiscard]] uint32_t CreateRtv(const D3D12_RENDER_TARGET_VIEW_DESC& rtvCreationDesc, const Microsoft::WRL::ComPtr<ID3D12Resource>& resource) const;
	[[nodiscard]] uint32_t CreateDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC& dsvCreationDesc, const Microsoft::WRL::ComPtr<ID3D12Resource>& resource) const;
    
    void Flush();
    
private:
    std::shared_ptr<Application> _app;
    std::shared_ptr<Camera> _camera;

    std::unique_ptr<GeometryPipeline> _geometryPipeline;
    std::unique_ptr<UIPipeline> _uiPipeline;

    UINT _width;
    UINT _height;
    float _aspectRatio;

    CD3DX12_VIEWPORT _viewport;
    CD3DX12_RECT _scissorRect;

	Microsoft::WRL::ComPtr<IDXGIFactory4> _factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device2> _device;

    std::unique_ptr<CommandQueue> _directCommandQueue;
    std::unique_ptr<CommandQueue> _copyCommandQueue;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> _bindlessRootSignature{};

    Microsoft::WRL::ComPtr<ID3D12Resource> _renderTargets[FRAME_COUNT];
	uint32_t _renderTargetIndex[FRAME_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Resource> _depthTarget;
	uint32_t _depthTargetIndex;

	std::unique_ptr<DescriptorHeap> _rtvHeap;
	std::unique_ptr<DescriptorHeap> _dsvHeap;
	std::unique_ptr<DescriptorHeap> _srvHeap;
	std::unique_ptr<DescriptorHeap> _samplerHeap;

    UINT _frameIndex;
    uint64_t _fenceValues[FRAME_COUNT] = {};
    const float clearColor[4] = { 255.0f / 255.0f, 182.0f / 255.0f, 193.0f / 255.0f, 1.0f }; // pink :)
    bool _useWarpDevice;

	void InitializeCore();
	void InitializeCommandQueues();
	void InitializeDescriptorHeaps();
    void InitializeSwapchainResources();

	void CreateRenderTargets();
	void CreateDepthTarget();
	void CreateBindlessRootSignature();

	void SetDescriptorHeaps(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList) const;
};