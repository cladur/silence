# Dependencies
find_package(glfw3 CONFIG REQUIRED)

#include(FetchContent)
#FetchContent_Declare(
#        fetch_vk_bootstrap
#        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
#        GIT_TAG v0.7 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
#)
#FetchContent_MakeAvailable(fetch_vk_bootstrap)

set(MANAGER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(MANAGER_DIR)

add_subdirectory(audio)
add_subdirectory(animation)
add_subdirectory(display)
add_subdirectory(ecs)
add_subdirectory(scene)
add_subdirectory(input)
add_subdirectory(font)
add_subdirectory(resource)
add_subdirectory(render)
add_subdirectory(physics)
add_subdirectory(ai)
add_subdirectory(gameplay)
