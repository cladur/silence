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
        text/text_draw.cpp text/text_draw.h
        debug/debug_draw.cpp debug/debug_draw.h
        ecs/render_system.cpp ecs/render_system.h
        ecs/model_instance.cpp ecs/model_instance.h
        )

# Include directories
target_include_directories(${OPENGL_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

# Link libraries
target_link_libraries(${OPENGL_MANAGER_NAME} PRIVATE
        silence_common
        silence_core
        silence_assets
        glad::glad
        ${ECS_MANAGER_NAME}
        )