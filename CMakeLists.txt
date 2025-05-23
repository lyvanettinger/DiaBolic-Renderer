cmake_minimum_required (VERSION 3.8)

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_BUILD_RPATH "$ORIGIN")

project( DialogueBox LANGUAGES CXX )

# Enable to build shared libraries.
option(BUILD_SHARED_LIBS "Create shared libraries." OFF)

project ("DialogueBox")

# Include sub-projects.
add_subdirectory("external")

add_subdirectory("DiaBolic")

# Add a custom target that always builds and runs the copy command
add_custom_target(copy-assets ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
		COMMENT "Copying assets into binary directory")