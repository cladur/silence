set(ANIMATION_MANAGER_NAME "silence_animation_manager")

add_library(${ANIMATION_MANAGER_NAME} STATIC
        animation_manager.cpp animation_manager.h
        ecs/animation_system.cpp ecs/animation_system.h
        ecs/animation_instance.cpp ecs/animation_instance.h)

target_include_directories(${ANIMATION_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${ANIMATION_MANAGER_NAME}
        PRIVATE
        silence_common
        silence_core
        )
