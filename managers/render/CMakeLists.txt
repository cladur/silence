set(OPENGL_MANAGER_NAME "silence_render_manager")

# Libraries
add_library(${OPENGL_MANAGER_NAME} STATIC
        render_manager.cpp render_manager.h
        render_system.cpp render_system.h
        mesh.cpp mesh.h
        shader.cpp shader.h
        model.cpp model.h
        texture.cpp texture.h
        material.cpp material.h
        render_pass.cpp render_pass.h
        text/text_draw.cpp text/text_draw.h
        debug/debug_draw.cpp debug/debug_draw.h
        skybox.cpp skybox.h
        utils.cpp utils.h
        framebuffer.cpp framebuffer.h
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