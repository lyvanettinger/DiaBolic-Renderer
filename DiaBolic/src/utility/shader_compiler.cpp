#include "utility/shader_compiler.hpp"

#include "utility/dx12_helpers.hpp"
#include "utility/log.hpp"

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

        ComPtr<IDxcBlob> Compile(const ShaderTypes& shaderType, const std::wstring_view shaderPath,
                       const std::wstring_view entryPoint)
        {
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

            std::vector<LPCWSTR> compilationArguments;

            // -E for the entry point (eg. 'main')
            compilationArguments.push_back(L"-E");
            compilationArguments.push_back(entryPoint.data());

            // -T for the target profile (eg. 'ps_6_6')
            compilationArguments.push_back(L"-T");
            compilationArguments.push_back(targetProfile.c_str());

            // -I for the target include directory
            compilationArguments.push_back(L"-I");
            compilationArguments.push_back(shaderDirectory.c_str());

            // Strip reflection data and pdbs (see later)
            compilationArguments.push_back(L"-Qstrip_debug");
            compilationArguments.push_back(L"-Qstrip_reflect");

            compilationArguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);

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

            const DxcBuffer sourceBuffer = {
                .Ptr = sourceBlob->GetBufferPointer(),
                .Size = sourceBlob->GetBufferSize(),
                .Encoding = 0u,
            };

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
                dblog::error(std::format("[SHADER COMPILER] Shader path: {}, Error: {}", wStringToString(shaderPath), errorMessage));
            }

            ComPtr<IDxcBlob> compiledShaderBlob{nullptr};
            ThrowIfFailed(compiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShaderBlob), nullptr));

            return compiledShaderBlob;
        }
    }
}