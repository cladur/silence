set(GAME_NAME "silence_game")

add_executable(${GAME_NAME}
        main.cpp
        game.cpp game.h)

# Define the link libraries
target_link_libraries(${GAME_NAME} PRIVATE
        silence_engine
        )