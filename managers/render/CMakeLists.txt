set(RENDER_MANAGER_NAME "silence_render_manager")

# Libraries
add_library(${RENDER_MANAGER_NAME} STATIC
        render_manager.cpp render_manager.h
        vk_initializers.cpp vk_initializers.h
        pipeline_builder.cpp pipeline_builder.h
        vk_types.h
        vk_mesh.cpp vk_mesh.h
        vk_textures.cpp vk_textures.h
        render_system.cpp render_system.h
        debug/debug_drawing.cpp debug/debug_drawing.h)

# Include directories
target_include_directories(${RENDER_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

# Link libraries

target_link_libraries(${RENDER_MANAGER_NAME} PRIVATE
        silence_common
        silence_core
        Vulkan::Vulkan
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
        vk-bootstrap::vk-bootstrap
        ${ECS_MANAGER_NAME}
        )