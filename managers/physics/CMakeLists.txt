set(PHYSICS_MANAGER_NAME "silence_physics_manager")

add_library(${PHYSICS_MANAGER_NAME} STATIC
        physics_manager.h physics_manager.cpp
        ecs/physics_system.cpp ecs/physics_system.h
        ecs/collision_system.cpp ecs/collision_system.h
        )

target_include_directories(${PHYSICS_MANAGER_NAME} PUBLIC ${MANAGER_DIR})


target_link_libraries(${PHYSICS_MANAGER_NAME}
        PRIVATE
        silence_common
        silence_core
        )

