set(EDITOR_NAME "silence_editor")

add_executable(${EDITOR_NAME}
        main.cpp
        )

# Define the link libraries
target_link_libraries(${EDITOR_NAME} PRIVATE
        silence_common
        silence_core
        silence_display_manager
        silence_ecs_manager
        silence_audio_manager
        silence_scene_manager
        silence_input_manager
        silence_font_manager
        )

if (USE_OPENGL)
    target_link_libraries(${EDITOR_NAME} PRIVATE silence_opengl_manager)
else ()
    target_link_libraries(${EDITOR_NAME} PRIVATE silence_render_manager)
endif ()