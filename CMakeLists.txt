cmake_minimum_required(VERSION 3.25)
project(silence)

set(CMAKE_CXX_STANDARD 20)

# We set the exe output directory to build directory, so that it'll be next to the DLLs.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(USE_OPENGL TRUE)
if (USE_OPENGL)
    add_compile_definitions(USE_OPENGL)
endif ()

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(thirdparty)
add_subdirectory(core)
add_subdirectory(managers)
add_subdirectory(asset_baker)
add_subdirectory(editor)
add_subdirectory(game)


# Commented out till we bring back Vulkan

## Shader compilation
#find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
#
#if (NOT GLSL_VALIDATOR)
#    message(FATAL_ERROR "glslangValidator not found!")
#endif ()
#
#file(GLOB_RECURSE GLSL_SOURCE_FILES
#        "${PROJECT_SOURCE_DIR}/resources/vk_shaders/*.frag"
#        "${PROJECT_SOURCE_DIR}/resources/vk_shaders/*.vert"
#        "${PROJECT_SOURCE_DIR}/resources/vk_shaders/*.comp"
#        )
#
#foreach (GLSL ${GLSL_SOURCE_FILES})
#    message(STATUS "BUILDING SHADER")
#    get_filename_component(FILE_NAME ${GLSL} NAME)
#    set(SPIRV "${PROJECT_SOURCE_DIR}/resources/vk_shaders/${FILE_NAME}.spv")
#    # message(STATUS ${GLSL})
#    # add_custom_command(
#    #         OUTPUT ${SPIRV}
#    #         COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
#    #         DEPENDS ${GLSL})
#
#    # add_custom_command didn't work for some reason, so we use execute_process instead
#    execute_process(
#            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
#            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    )
#    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
#endforeach (GLSL)
#
#add_custom_target(
#        silence_shaders
#        DEPENDS ${SPIRV_BINARY_FILES}
#)