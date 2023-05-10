set(OPENGL_MANAGER_NAME "silence_render_manager")

# Libraries
add_library(${OPENGL_MANAGER_NAME} STATIC
        render_manager.cpp render_manager.h
        render_scene.cpp render_scene.h
        common/mesh.cpp common/mesh.h
        common/shader.cpp common/shader.h
        common/model.cpp common/model.h
        common/texture.cpp common/texture.h
        common/material.cpp common/material.h
        common/render_pass.cpp common/render_pass.h
        common/skybox.cpp common/skybox.h
        common/utils.cpp common/utils.h
        common/framebuffer.cpp common/framebuffer.h
        common/frustum.cpp common/frustum.h
        ecs/render_system.cpp ecs/render_system.h
        ecs/frustum_draw_system.cpp ecs/frustum_draw_system.h
        ecs/model_instance.cpp ecs/model_instance.h
        debug/debug_draw.cpp debug/debug_draw.h
        transparent_elements/transparent_object.h
        transparent_elements/text/text_draw.cpp transparent_elements/text/text_draw.h
        transparent_elements/ui/sprite_manager.cpp transparent_elements/ui/sprite_manager.h
        transparent_elements/ui/sprite_draw.cpp transparent_elements/ui/sprite_draw.h
        transparent_elements/ui/ui_elements/ui_slider.cpp transparent_elements/ui/ui_elements/ui_slider.h
        transparent_elements/ui/ui_elements/ui_element.cpp transparent_elements/ui/ui_elements/ui_element.h
        transparent_elements/ui/ui_elements/ui_anchor.cpp transparent_elements/ui/ui_elements/ui_anchor.h
        transparent_elements/ui/ui_elements/ui_image.cpp transparent_elements/ui/ui_elements/ui_image.h
        transparent_elements/ui/ui_elements/ui_text.cpp transparent_elements/ui/ui_elements/ui_text.h
        transparent_elements/ui/ui_elements/ui_button.cpp transparent_elements/ui/ui_elements/ui_button.h common/mesh_bounding_sphere.cpp common/mesh_bounding_sphere.h
        transparent_elements/ui_manager.h 
        transparent_elements/ui_manager.cpp)

# Include directories
target_include_directories(${OPENGL_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

# Link libraries
find_package(glad CONFIG REQUIRED)

target_link_libraries(${OPENGL_MANAGER_NAME}
        PUBLIC
        # TODO: Remove these dependencies
        fmod
        PRIVATE
        silence_input_manager
        silence_audio_manager

        silence_common
        silence_core
        silence_assets
        glad::glad
        ${ECS_MANAGER_NAME}
        )