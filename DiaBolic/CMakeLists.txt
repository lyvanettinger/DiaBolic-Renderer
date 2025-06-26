cmake_minimum_required (VERSION 3.8)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES inc/*.hpp)

add_executable( DiaBolic
    ${HEADER_FILES}
    ${SRC_FILES}
	src/pch.cpp
)

set_property(TARGET DiaBolic
		PROPERTY CXX_STANDARD 20
)

add_dependencies( DiaBolic copy-assets)

target_link_libraries( DiaBolic PUBLIC External d3d12.lib dxgi.lib D3DCompiler.lib dxcompiler.lib dxguid.lib)
target_include_directories( DiaBolic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}../assets/shaders)

target_precompile_headers( DiaBolic
	PRIVATE "src/pch.h")

# Set Local Debugger Settings (Command Arguments and Environment Variables)
set( COMMAND_ARGUMENTS "-wd \"${CMAKE_SOURCE_DIR}\"" )

# Surpress C++17 deprecation warnings that clutter console
target_compile_definitions(DiaBolic PUBLIC
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

# Make use of DX12 Agility SDK
if(TARGET Microsoft::DirectX12-Agility)
    file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12")
    add_custom_command(TARGET DiaBolic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:DiaBolic>/D3D12
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:DiaBolic>/D3D12
       COMMAND_EXPAND_LISTS
    )
endif()