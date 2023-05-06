set(ANIMATION_MANAGER_NAME "silence_animation_manager")

add_library(${ANIMATION_MANAGER_NAME} STATIC
        animation.cpp animation.h
        animation_manager.h animation_manager.cpp
        channel.cpp channel.h
        )

target_include_directories(${ANIMATION_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${ANIMATION_MANAGER_NAME}
        PUBLIC
        fmod
        PRIVATE
        silence_common
        silence_core
        silence_assets
        glad::glad
        ${ECS_MANAGER_NAME}
        ${OPENGL_MANAGER_NAME}
        )
