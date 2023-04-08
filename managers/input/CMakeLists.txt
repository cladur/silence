set(INPUT_MANAGER_NAME "silence_input_manager")

add_library(${INPUT_MANAGER_NAME} STATIC
        input_manager.cpp input_manager.h
        input_key.h
        )

target_include_directories(${INPUT_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${INPUT_MANAGER_NAME}
        PRIVATE
        glfw
        silence_common
        silence_core
        )
