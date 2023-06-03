set(DISPLAY_MANAGER_NAME "silence_display_manager")

add_library(${DISPLAY_MANAGER_NAME} STATIC
        display_manager.cpp display_manager.h
        )

target_include_directories(${DISPLAY_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${DISPLAY_MANAGER_NAME}
        PUBLIC
        PRIVATE
        silence_common
        glfw
        )