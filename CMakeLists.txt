cmake_minimum_required(VERSION 3.10)
project(VulkanModNative)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_BUILD_TYPE Debug)


add_library(shader_loader)
target_include_directories(shader_loader PUBLIC include)

# Set the library name and source files
set(EXEC_NAME shader_compiler)
file(GLOB_RECURSE SRC_FILES "src/compiler/*.cpp" "lib/*.c*")

find_package(Vulkan REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SHADERC REQUIRED shaderc)

# Create the shared library
add_executable(${EXEC_NAME} ${SRC_FILES})

include_directories(
    lib/
    include/
    ${Vulkan_INCLUDE_DIRS}
    ${SHADERC_INCLUDE_DIRS}
)

target_link_libraries(${EXEC_NAME} 
    ${Vulkan_LIBRARIES} 
    ${SHADERC_LIBRARIES}
)