cmake_minimum_required (VERSION 3.8)

# Set W4
string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/DiaBolic) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/DiaBolic) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/DiaBolic) # Output directory for executables (.EXE)

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