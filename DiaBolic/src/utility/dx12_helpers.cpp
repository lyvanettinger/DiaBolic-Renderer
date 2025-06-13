#include "utility/dx12_helpers.hpp"

#include "utility/log.hpp"

void Util::GetHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

    Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

void Util::CheckFeatureSupport(const Microsoft::WRL::ComPtr<ID3D12Device>& device)
{
    {   // Feature Level
        D3D12_FEATURE_DATA_FEATURE_LEVELS cap{};
        cap.NumFeatureLevels = 1;
        if (SUCCEEDED(device->CheckFeatureSupport(
            D3D12_FEATURE_FEATURE_LEVELS,
            &cap,
            sizeof(cap))))
        {
            std::string level;
            switch(cap.MaxSupportedFeatureLevel)
            {
            case D3D_FEATURE_LEVEL_1_0_CORE:
                level = "1_0_CORE";
                break;
            case D3D_FEATURE_LEVEL_9_1:
                level = "9_1";
                break;
            case D3D_FEATURE_LEVEL_9_2:
                level = "9_2";
                break;
            case D3D_FEATURE_LEVEL_9_3:
                level = "9_3";
                break;
            case D3D_FEATURE_LEVEL_10_0:
                level = "10_0";
                break;
            case D3D_FEATURE_LEVEL_10_1:
                level = "10_1";
                break;
            case D3D_FEATURE_LEVEL_11_0:
                level = "11_0";
                break;
            case D3D_FEATURE_LEVEL_11_1:
                level = "11_1";
                break;
            case D3D_FEATURE_LEVEL_12_0:
                level = "12_0";
                break;
            case D3D_FEATURE_LEVEL_12_1:
                level = "12_1";
                break;
            case D3D_FEATURE_LEVEL_12_2:
                level = "12_2";
            }
            dblog::info("[DEVICE] Supported Feature Level: {}", level.c_str());
        }
    }

    {   // Shader Model
        D3D12_FEATURE_DATA_SHADER_MODEL cap{};
        if (SUCCEEDED(device->CheckFeatureSupport(
            D3D12_FEATURE_FEATURE_LEVELS,
            &cap,
            sizeof(cap))))
        {
            std::string model;
            switch(cap.HighestShaderModel)
            {
            case D3D_SHADER_MODEL_5_1:
                model = "5_1";
                break;
            case D3D_SHADER_MODEL_6_0:
                model = "6_0";
                break;
            case D3D_SHADER_MODEL_6_1:
                model = "6_1";
                break;
            case D3D_SHADER_MODEL_6_2:
                model = "6_2";
                break;
            case D3D_SHADER_MODEL_6_3:
                model = "6_3";
                break;
            case D3D_SHADER_MODEL_6_4:
                model = "6_4";
                break;
            case D3D_SHADER_MODEL_6_5:
                model = "6_5";
                break;
            case D3D_SHADER_MODEL_6_6:
                model = "6_6";
                break;
            case D3D_SHADER_MODEL_6_7:
                model = "6_7";
            }
            dblog::info("[DEVICE] Supported Shader Model: {}", model.c_str());
        }
    }
}