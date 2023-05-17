set(ECS_MANAGER_NAME "silence_ecs_manager")

add_library(${ECS_MANAGER_NAME} STATIC
        entity_manager.cpp entity_manager.h
        component_array.h component_array_interface.h
        component_manager.h
        system_manager.h
        world.cpp world.h
        systems/base_system.h
        systems/physics_system.cpp systems/physics_system.h
        systems/collider_components_factory.h
        systems/isolated_entities_system.cpp systems/isolated_entities_system.h
        systems/root_parent_system.cpp systems/root_parent_system.h
        component_visitor.h 
        systems/collision_system.cpp systems/collision_system.h 
        systems/collider_draw.cpp systems/collider_draw.h
        systems/light_system.cpp systems/light_system.h
        systems/agent_system.cpp systems/agent_system.h
        systems/enemy_pathing.cpp systems/enemy_pathing.h systems/enemy_path_draw_system.cpp systems/enemy_path_draw_system.h)

target_include_directories(${ECS_MANAGER_NAME} PUBLIC ${MANAGER_DIR})


target_link_libraries(${ECS_MANAGER_NAME} PRIVATE
        silence_core
        silence_common
        )