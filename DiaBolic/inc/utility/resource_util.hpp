#pragma once

namespace Util
{
	void CreateCube(std::vector<DirectX::XMFLOAT3>& vertices,
		std::vector<DirectX::XMFLOAT3>& normals,
		std::vector<DirectX::XMFLOAT2>& uvs,
		std::vector<uint16_t>& indices, float size);

	void LoadBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, 
		size_t numElements, size_t elementSize, const void* bufferData, 
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	void LoadTextureFromFile(Microsoft::WRL::ComPtr<ID3D12Device> device, 
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		const std::wstring& filePath, DXGI_FORMAT& format);
	
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, 
		Microsoft::WRL::ComPtr<ID3D12Resource> resource, 
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
}