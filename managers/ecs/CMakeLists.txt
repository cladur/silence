set(ECS_MANAGER_NAME "silence_ecs_manager")

add_library(${ECS_MANAGER_NAME} STATIC
        entity_manager.cpp entity_manager.h
        component_array.h component_array_interface.h
        component_manager.h
        system_manager.h
        world.cpp world.h
        systems/base_system.h
        systems/collider_components_factory.h
        systems/isolated_entities_system.cpp systems/isolated_entities_system.h
        systems/root_parent_system.cpp systems/root_parent_system.h
        component_visitor.h
        systems/collider_draw.cpp systems/collider_draw.h
        systems/agent_system.cpp systems/agent_system.h
        systems/agent_movement_system.cpp systems/agent_movement_system.h
        systems/hacker_system.cpp systems/hacker_system.h
        systems/hacker_movement_system.cpp systems/hacker_movement_system.h
        systems/enemy_pathing.cpp systems/enemy_pathing.h systems/enemy_path_draw_system.cpp systems/enemy_path_draw_system.h
        systems/interactable_system.cpp systems/interactable_system.h
        systems/platform_system.cpp
        systems/platform_system.h
        systems/enemy_system.cpp
        systems/enemy_system.h
        systems/taggable_system.cpp
        systems/taggable_system.h
        systems/show_taggable_system.cpp
        systems/show_taggable_system.h
        systems/highlight_system.h
        systems/highlight_system.cpp
        systems/detection_camera_system.h
        systems/detection_camera_system.cpp systems/cable_system.cpp systems/cable_system.h
        systems/rotator_system.cpp systems/rotator_system.h
        systems/light_switcher_system.cpp systems/light_switcher_system.h
        )

target_include_directories(${ECS_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

find_package(OpenMP)

target_link_libraries(${ECS_MANAGER_NAME}
        PUBLIC
        fmod
        PRIVATE
        silence_core
        silence_common
        OpenMP::OpenMP_CXX
        )