set(RENDER_SERVER_NAME "silence_render_manager")
set(DISPLAY_SERVER_NAME "silence_display_manager")
set(ECS_MANAGER_NAME "silence_ecs_manager")

add_library(${RENDER_SERVER_NAME} STATIC render_manager.cpp render_manager.h rendering/vk_initializers.cpp rendering/vk_initializers.h)
add_library(${DISPLAY_SERVER_NAME} STATIC display_manager.cpp display_manager.h)
add_library(${ECS_MANAGER_NAME} STATIC ecs/entity_manager.cpp ecs/entity_manager.h ecs/component_array.h ecs/component_array_interface.h ecs/component_manager.h ecs/base_system.h ecs/system_manager.h ecs/ecs_manager.h systems/physics_system.cpp systems/physics_system.h ../core/components/transform_component.h ../core/components/rigidbody_component.h ../core/components/gravity_component.h)

target_include_directories(${RENDER_SERVER_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${DISPLAY_SERVER_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${ECS_MANAGER_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${RENDER_SERVER_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${DISPLAY_SERVER_NAME} PUBLIC glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${ECS_MANAGER_NAME} PRIVATE glm::glm)

find_package(Vulkan REQUIRED)
target_link_libraries(${RENDER_SERVER_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${DISPLAY_SERVER_NAME} PUBLIC Vulkan::Vulkan)

include(FetchContent)
FetchContent_Declare(
        fetch_vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG v0.7 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
target_link_libraries(${RENDER_SERVER_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
target_link_libraries(${DISPLAY_SERVER_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
