# Dependencies
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
        fetch_vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG v0.7 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)

set(MANAGER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(MANAGER_DIR)

add_subdirectory(audio)
add_subdirectory(display)
add_subdirectory(ecs)
add_subdirectory(scene)
add_subdirectory(input)
add_subdirectory(font)
add_subdirectory(render)
