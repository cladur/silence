set(GAMEPLAY_MANAGER_NAME "silence_gameplay_manager")

add_library(${GAMEPLAY_MANAGER_NAME} STATIC
        gameplay_manager.cpp gameplay_manager.h)

target_include_directories(${GAMEPLAY_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

find_package(OpenMP)

target_link_libraries(${GAMEPLAY_MANAGER_NAME}
        PUBLIC
        fmod
        PRIVATE
        silence_common
        silence_core
        OpenMP::OpenMP_CXX
        )
