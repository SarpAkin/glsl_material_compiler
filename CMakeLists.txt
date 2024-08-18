cmake_minimum_required(VERSION 3.10)
project(VulkanModNative)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_BUILD_TYPE Debug)


# add_library(shader_loader)
# target_include_directories(shader_loader PUBLIC include)

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





# Example usage of the run_shader_compiler function in another CMake file:
#   include(path/to/this/CMakeLists.txt)
#   run_shader_compiler(OUTPUT=output.bin material.json)

# Call the run_shader_compiler function with the specified output file and input file
function(run_shader_compiler)
    # Parse the arguments
    set(oneValueArgs OUTPUT) # Required argument
    set(multiValueArgs) # Optional arguments
    cmake_parse_arguments(RSC "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message("MultiValueArgs: ${RSC_UNPARSED_ARGUMENTS}")

    # Check if the output file was specified
    if(NOT DEFINED RSC_OUTPUT)
        # If not, print an error message and stop the build
        message(FATAL_ERROR "Output file not specified")
    endif()

    # Create the custom target
    add_custom_target(${RSC_OUTPUT}
        # Run the application with the specified arguments
        COMMAND ${EXEC_NAME} ${RSC_UNPARSED_ARGUMENTS} -o ${RSC_OUTPUT}
        # Depend on the application executable
        DEPENDS ${EXEC_NAME}
        # Print a comment indicating that the application is being run
        COMMENT "Running shader compiler"
        # Do not preprocess the command by default
        VERBATIM
    )
endfunction()

