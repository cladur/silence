set(GAME_NAME "silence_game")

add_executable(${GAME_NAME}
        main.cpp
        game.cpp
        game.h
        menu_test.cpp
        menu_test.h)

find_package(OpenMP)

target_link_libraries(${GAME_NAME} PRIVATE
        silence_engine
        OpenMP::OpenMP_CXX
        )
