set(RENDER_MANAGER_NAME "silence_render_manager")
set(DISPLAY_MANAGER_NAME "silence_display_manager")
set(ECS_MANAGER_NAME "silence_ecs_manager")

add_library(${RENDER_MANAGER_NAME} STATIC render_manager.cpp render_manager.h rendering/vk_initializers.cpp rendering/vk_initializers.h rendering/pipeline_builder.cpp rendering/pipeline_builder.h rendering/vk_types.h rendering/vk_mesh.cpp rendering/vk_mesh.h rendering/vk_textures.cpp rendering/vk_textures.h rendering/render_system.cpp rendering/render_system.h rendering/render_system.h)
add_library(${DISPLAY_MANAGER_NAME} STATIC display_manager.cpp display_manager.h)
add_library(${ECS_MANAGER_NAME} STATIC ecs/entity_manager.cpp ecs/entity_manager.h ecs/component_array.h ecs/component_array_interface.h ecs/component_manager.h systems/base_system.h ecs/system_manager.h ecs/ecs_manager.h physics/physics_system.cpp physics/physics_system.h ../core/components/transform_component.h ../core/components/rigidbody_component.h ../core/components/gravity_component.h systems/parent_system.cpp systems/parent_system.h ecs/parent_manager.cpp ecs/parent_manager.h systems/isolated_entities_system.cpp systems/isolated_entities_system.h systems/root_parent_system.cpp systems/root_parent_system.h physics/collider_components_factory.h)

target_include_directories(${RENDER_MANAGER_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${DISPLAY_MANAGER_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${ECS_MANAGER_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${RENDER_MANAGER_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(${ECS_MANAGER_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${DISPLAY_MANAGER_NAME} PUBLIC glfw)

find_package(Vulkan REQUIRED)
target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${DISPLAY_MANAGER_NAME} PUBLIC Vulkan::Vulkan)

find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC unofficial::vulkan-memory-allocator::vulkan-memory-allocator)

find_package(unofficial-vulkan-memory-allocator-hpp CONFIG REQUIRED)
target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC glm::glm)
target_link_libraries(${ECS_MANAGER_NAME} PRIVATE glm::glm)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${RENDER_MANAGER_NAME} PRIVATE imgui::imgui)

include(FetchContent)
FetchContent_Declare(
        fetch_vk_bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG v0.7 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
target_link_libraries(${DISPLAY_MANAGER_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
target_link_libraries(${ECS_MANAGER_NAME} PRIVATE silence_core)

target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC silence_core)
target_link_libraries(${RENDER_MANAGER_NAME} PUBLIC ${ECS_MANAGER_NAME})