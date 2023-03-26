set(GAME_NAME "silence_game")

# Add source files
file(GLOB_RECURSE SOURCE_FILES_EXE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Add header files
file(GLOB_RECURSE HEADER_FILES_EXE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_executable(${GAME_NAME} main.cpp)

# Define the link libraries
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${GAME_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${GAME_NAME} PRIVATE magic_enum::magic_enum)

target_link_libraries(${GAME_NAME} PRIVATE silence_display_manager silence_render_manager)

target_link_libraries(${GAME_NAME} PRIVATE silence_core)