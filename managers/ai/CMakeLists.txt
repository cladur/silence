set(AI_MANAGER_NAME "silence_ai_manager")

add_library(${AI_MANAGER_NAME} STATIC
        state_machine/state.h
        state_machine/state_machine.h
        state_machine/state_machine.cpp
        state_machine/states/enemy/enemy_patrolling.cpp
        state_machine/states/enemy/enemy_patrolling.h
        state_machine/states/enemy/enemy_looking.cpp
        state_machine/states/enemy/enemy_looking.h
        state_machine/states/enemy/enemy_fully_aware.cpp state_machine/states/enemy/enemy_fully_aware.h state_machine/states/enemy/enemy_utils.h state_machine/states/enemy/enemy_stationary_patrolling.cpp state_machine/states/enemy/enemy_stationary_patrolling.h state_machine/states/enemy/enemy_dead.cpp state_machine/states/enemy/enemy_dead.h state_machine/states/enemy/enemy_distracted.cpp state_machine/states/enemy/enemy_distracted.h)

target_include_directories(${AI_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${AI_MANAGER_NAME}
        PUBLIC
        fmod
        PRIVATE
        silence_common
        silence_core
        )
