set(SCENE_MANAGER_NAME "silence_scene_manager")

add_library(${SCENE_MANAGER_NAME} STATIC
        scene_manager.cpp scene_manager.h
        )

target_include_directories(${SCENE_MANAGER_NAME} PUBLIC ${MANAGER_DIR})


target_link_libraries(${SCENE_MANAGER_NAME}
        PRIVATE
        silence_common
        silence_core
        )

