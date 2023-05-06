set(RESOURCE_MANAGER_NAME "silence_resource_manager")

add_library(${RESOURCE_MANAGER_NAME} STATIC
        resource_manager.cpp resource_manager.h
        )

target_include_directories(${RESOURCE_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${RESOURCE_MANAGER_NAME}
        PRIVATE
        glfw
        silence_common
        silence_core
        )
