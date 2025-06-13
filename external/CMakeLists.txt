## FETCH CONTENTS
include(FetchContent)

# Spdlog
FetchContent_Declare(
        SpdLog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SpdLog)


## SUB DIRECTORIES

# Make sure cmake can find vcpkg installed packages
include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

# DirectXMath
find_package(DirectXMath CONFIG REQUIRED)

# DirectXTex
find_package(DirectXTex CONFIG REQUIRED)

# GLFW
find_package( OpenGL REQUIRED )

include_directories( ${OPENGL_INCLUDE_DIRS} )

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
set( GLAD_GL "" )

add_subdirectory("glfw")

option( GLFW-CMAKE-STARTER-USE-GLFW-GLAD "Use GLAD from GLFW" ON )

if( GLFW-CMAKE-STARTER-USE-GLFW-GLAD )
    include_directories("${GLFW_SOURCE_DIR}/deps")
    set( GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
endif()

# Assimp
add_subdirectory("assimp")

add_library(External INTERFACE)
target_link_libraries(External INTERFACE Microsoft::DirectXTex Microsoft::DirectXMath glfw ${OPENGL_LIBRARIES} assimp spdlog::spdlog)