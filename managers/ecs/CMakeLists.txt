set(ECS_MANAGER_NAME "silence_ecs_manager")

add_library(${ECS_MANAGER_NAME} STATIC
        entity_manager.cpp entity_manager.h
        component_array.h component_array_interface.h
        component_manager.h
        system_manager.h
        ecs_manager.cpp ecs_manager.h
        systems/base_system.h
        systems/physics_system.cpp systems/physics_system.h
        systems/parent_system.cpp systems/parent_system.h
        systems/isolated_entities_system.cpp systems/isolated_entities_system.h
        systems/root_parent_system.cpp systems/root_parent_system.h
        component_visitor.h)

target_include_directories(${ECS_MANAGER_NAME} PUBLIC ${MANAGER_DIR})


target_link_libraries(${ECS_MANAGER_NAME} PRIVATE
        silence_core
        silence_common
        )