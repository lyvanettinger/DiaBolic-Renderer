#pragma once

namespace Util
{
    enum class ShaderTypes : uint8_t
    {
        Vertex,
        Pixel,
        Compute,
        RootSignature,
    };

    namespace ShaderCompiler
    {
        [[nodiscard]] Microsoft::WRL::ComPtr<IDxcBlob> Compile(const ShaderTypes& shaderType, const std::wstring_view shaderPath,
                                     const std::wstring_view entryPoint);
    }
}

