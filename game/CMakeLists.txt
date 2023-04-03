set(GAME_NAME "silence_game")

add_executable(${GAME_NAME}
        main.cpp
        )

# Define the link libraries
target_link_libraries(${GAME_NAME} PRIVATE
        silence_common
        silence_core
        silence_display_manager
        silence_render_manager
        silence_ecs_manager
        silence_audio_manager
        )