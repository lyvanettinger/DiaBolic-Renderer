#include "shader_compiler.hpp"

#include "dx12_helpers.hpp"

using namespace Microsoft::WRL;

namespace Util
{
    namespace ShaderCompiler
    {
        // Responsible for the actual compilation of shaders.
    ComPtr<IDxcCompiler3> compiler{};

    // Used to create include handle and provides interfaces for loading shader to blob, etc.
    ComPtr<IDxcUtils> utils{};
    ComPtr<IDxcIncludeHandler> includeHandler{};

    std::wstring shaderDirectory{};

    Shader Compile(const ShaderTypes& shaderType, const std::wstring shaderPath,
                   const std::wstring entryPoint, const bool extractRootSignature)
    {
        Shader shader{};

        if (!utils)
        {
            ThrowIfFailed(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
            ThrowIfFailed(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
            ThrowIfFailed(utils->CreateDefaultIncludeHandler(&includeHandler));

            shaderDirectory = L"assets/shaders";
        }

        // Setup compilation arguments.
        const std::wstring targetProfile = [=]() {
            switch (shaderType)
            {
            case ShaderTypes::Vertex: {
                return L"vs_6_6";
            }
            break;

            case ShaderTypes::Pixel: {
                return L"ps_6_6";
            }
            break;

            case ShaderTypes::Compute: {
                return L"cs_6_6";
            }
            break;

            default: {
                return L"";
            }
            break;
            }
        }();

        std::vector<LPCWSTR> compilationArguments = {
            L"-HV",
            L"2021",
            L"-E",
            entryPoint.data(),
            L"-T",
            targetProfile.c_str(),
            DXC_ARG_PACK_MATRIX_ROW_MAJOR,
            DXC_ARG_WARNINGS_ARE_ERRORS,
            DXC_ARG_ALL_RESOURCES_BOUND,
        };

        // Indicate that the shader should be in a debuggable state if in debug mode.
        // Else, set optimization level to 03.
#ifdef _DEBUG
            compilationArguments.push_back(DXC_ARG_DEBUG);
#else
            compilationArguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

        // Load the shader source file to a blob.
        ComPtr<IDxcBlobEncoding> sourceBlob{nullptr};
        ThrowIfFailed(utils->LoadFile(shaderPath.data(), nullptr, &sourceBlob));

        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();
        sourceBuffer.Encoding = 0u;

        // Compile the shader.
        ComPtr<IDxcResult> compiledShaderBuffer{};
        ThrowIfFailed(compiler->Compile(&sourceBuffer, compilationArguments.data(),
                                             static_cast<uint32_t>(compilationArguments.size()), includeHandler.Get(),
                                             IID_PPV_ARGS(&compiledShaderBuffer)));

        // Get compilation errors (if any).
        ComPtr<IDxcBlobUtf8> errors{};
        ThrowIfFailed(compiledShaderBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
        if (errors && errors->GetStringLength() > 0)
        {
            const LPCSTR errorMessage = errors->GetStringPointer();
            std::printf("Shader path : {}, Error : {}", shaderPath, errorMessage);
        }

        ComPtr<IDxcBlob> compiledShaderBlob{nullptr};
        compiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShaderBlob), nullptr);

        shader.shaderBlob = compiledShaderBlob;

        ComPtr<IDxcBlob> rootSignatureBlob{nullptr};
        if (extractRootSignature)
        {
            compiledShaderBuffer->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&rootSignatureBlob), nullptr);
            shader.rootSignatureBlob = rootSignatureBlob;
        }

        return shader;
    }
    }
}