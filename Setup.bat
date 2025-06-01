powershell -Command "Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.614.1 -OutFile agility.zip"
powershell -Command "& {Expand-Archive agility.zip external/DirectXAgilitySDK}"

xcopy External\DirectXAgilitySDK\build\native\bin\x64\* build\x64-debug\DiaBolic\D3D12\
xcopy External\DirectXAgilitySDK\build\native\bin\x64\* build\x64-release\DiaBolic\D3D12\

powershell -Command "Invoke-WebRequest -Uri https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip -OutFile dxc.zip"
powershell -Command "& {Expand-Archive dxc.zip external/DirectXShaderCompiler}"

xcopy External\DirectXShaderCompiler\bin\x64\* build\x64-debug\DiaBolic\
xcopy External\DirectXShaderCompiler\bin\x64\* build\x64-release\DiaBolic\

xcopy External\DirectXShaderCompiler\lib\x64\* build\x64-debug\DiaBolic\
xcopy External\DirectXShaderCompiler\lib\x64\* build\x64-release\DiaBolic\