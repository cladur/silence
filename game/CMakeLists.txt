set(GAME_NAME "silence_game")

add_executable(${GAME_NAME}
        main.cpp
        menu_demo.cpp menu_demo.h)

# Define the link libraries
target_link_libraries(${GAME_NAME} PRIVATE
        silence_common
        silence_core
        silence_display_manager
        silence_ecs_manager
        silence_audio_manager
        silence_scene_manager
        silence_input_manager
        silence_font_manager
        )


if(USE_OPENGL)
        target_link_libraries(${GAME_NAME} PRIVATE silence_opengl_manager)
else()
        target_link_libraries(${GAME_NAME} PRIVATE silence_render_manager)
endif()