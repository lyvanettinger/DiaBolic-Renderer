## FETCH CONTENTS

# Spdlog

include(FetchContent)

FetchContent_Declare(
        SpdLog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SpdLog)

## SUB DIRECTORIES

add_subdirectory("DirectXTex")

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory("glfw")

add_library(External INTERFACE)
target_link_libraries(External INTERFACE DirectXTex glfw spdlog::spdlog)