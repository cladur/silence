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
        ui/text/font_manager.cpp
        ui/text/font_manager.h
        ui/text/font.cpp
        ui/text/font.h
        ui/ui_render_system.cpp
        ui/ui_render_system.h
        debug/debug_drawing.cpp debug/debug_drawing.h
        ../animation/skinned_mesh.cpp ../animation/skinned_mesh.h ../animation/skinned_model.cpp ../animation/skinned_model.h ../animation/animation.cpp ../animation/animation.h ../animation/joint.h ../animation/bone.cpp ../animation/bone.h ../animation/bone.cpp ../animation/bone.cpp ../animation/bone.cpp ../animation/animator.cpp ../animation/animator.h ../animation/bone.cpp)

find_package(Freetype CONFIG REQUIRED)
if(TARGET freetype AND NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()
if(NOT TARGET Freetype::Freetype)
    MESSAGE("FREETYPE NOT FOUND!")
endif()
target_link_libraries(${RENDER_MANAGER_NAME} PRIVATE Freetype::Freetype)

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