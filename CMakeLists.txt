cmake_minimum_required(VERSION 3.10)

# Project name and language
project(blorp CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SDLTTF_VENDORED ON)

add_subdirectory(vendor/SDL3) 
add_subdirectory(vendor/SDL3_image)
add_subdirectory(vendor/SDL3_ttf)

add_executable(${PROJECT_NAME} src/main.cpp)

# Link SDL3 
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)
target_link_libraries(${PROJECT_NAME} PRIVATE m)

# Optionally, you can include SDL3 headers directly in your target
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_INCLUDE_DIRS})

