set(OPENGL_MANAGER_NAME "silence_opengl_manager")

# Libraries
add_library(${OPENGL_MANAGER_NAME} STATIC
        opengl_manager.cpp opengl_manager.h
        opengl_system.cpp opengl_system.h
        mesh.cpp mesh.h
        shader.cpp shader.h
        model.cpp model.h
        texture.cpp texture.h
        material.cpp material.h
        render_pass.cpp render_pass.h
        transparent_elements/text/text_draw.cpp transparent_elements/text/text_draw.h
        transparent_elements/ui/sprite_manager.cpp transparent_elements/ui/sprite_manager.h transparent_elements/ui/sprite_draw.cpp transparent_elements/ui/sprite_draw.h
        debug/debug_draw.cpp debug/debug_draw.h
        transparent_elements/ui/ui_elements/ui_slider.cpp transparent_elements/ui/ui_elements/ui_slider.h transparent_elements/ui/ui_elements/ui_element.cpp transparent_elements/ui/ui_elements/ui_element.h transparent_elements/ui/ui_elements/ui_anchor.cpp transparent_elements/ui/ui_elements/ui_anchor.h transparent_elements/ui/ui_elements/ui_image.cpp transparent_elements/ui/ui_elements/ui_image.h transparent_elements/transparent_draw.cpp transparent_elements/transparent_draw.h transparent_elements/ui/ui_elements/ui_text.cpp transparent_elements/ui/ui_elements/ui_text.h transparent_elements/ui/ui_elements/ui_button.cpp transparent_elements/ui/ui_elements/ui_button.h)

# Include directories
target_include_directories(${OPENGL_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

# Link libraries
find_package(glad CONFIG REQUIRED)

target_link_libraries(${OPENGL_MANAGER_NAME}
        PUBLIC
        fmod
        PRIVATE
        silence_common
        silence_core
        silence_assets
        glad::glad
        ${ECS_MANAGER_NAME}
        )