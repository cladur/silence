set(RENDER_MANAGER_NAME "silence_render_manager")

# Libraries
add_library(${RENDER_MANAGER_NAME} STATIC
        render_manager.cpp render_manager.h
        vk_initializers.cpp vk_initializers.h
        material_system.cpp material_system.h
        vk_types.h
        vk_mesh.cpp vk_mesh.h
        vk_textures.cpp vk_textures.h
        render_system.cpp render_system.h
        vk_descriptors.cpp vk_descriptors.h
        vk_shaders.cpp vk_shaders.h
        vk_scene.cpp vk_scene.h
        vk_push_buffer.cpp vk_push_buffer.h
        vk_scene_render.cpp)

# Include directories
target_include_directories(${RENDER_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

find_package(unofficial-spirv-reflect CONFIG REQUIRED)

# Link libraries

target_link_libraries(${RENDER_MANAGER_NAME} PRIVATE
        silence_common
        silence_core
        silence_assets
        Vulkan::Vulkan
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
        vk-bootstrap::vk-bootstrap
        unofficial::spirv-reflect::spirv-reflect
        ${ECS_MANAGER_NAME}
        )