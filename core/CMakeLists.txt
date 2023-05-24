set(COMMON_LIB_NAME "silence_common")
set(CORE_LIB_NAME "silence_core")

add_library(${COMMON_LIB_NAME} INTERFACE
        pch.h
        types.h)

add_library(${CORE_LIB_NAME} STATIC
        components/transform_component.h
        components/rigidbody_component.h
        components/parent_component.h
        components/children_component.h
        components/fmod_listener_component.h
        ../managers/ai/state_machine/state.h
        ../managers/ai/state_machine/state_machine.cpp
        ../managers/ai/state_machine/state_machine.h
        behavior_tree/tree_node.h
        behavior_tree/leaf_node.h
        behavior_tree/composite_node.h
        behavior_tree/decorator_node.h
        behavior_tree/behavior_tree.h
        behavior_tree/behavior_tree_builder.h
        behavior_tree/composite_nodes/sequence_node.h
        behavior_tree/leaf_nodes/success_leaf.h
        behavior_tree/leaf_nodes/failure_leaf.h
        behavior_tree/composite_nodes/selector_node.h
        behavior_tree/decorator_nodes/repeater_node.h
        behavior_tree/decorator_nodes/retry_node.h
        behavior_tree/decorator_nodes/invert_node.h
        serialization.h
        debug_camera/debug_camera.cpp debug_camera/debug_camera.h
        components/collider_tag_component.h
        components/static_tag_component.h
        components/collider_aabb.h
        components/collider_sphere.h
        components/collider_obb.h
        components/collider_sphere.h
        components/collider_obb.h
        components/enemy_path_component.h
        components/interactable_component.h
        cvars/cvars.cpp
        cvars/cvars.h
        components/attachment_component.h
        components/enemy_data_component.h)

target_include_directories(${CORE_LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Linking libraries
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(${COMMON_LIB_NAME} INTERFACE
        spdlog::spdlog spdlog::spdlog_header_only
        imgui::imgui
        glm::glm
        magic_enum::magic_enum
        Tracy::TracyClient
        nlohmann_json::nlohmann_json
        )

target_link_libraries(${CORE_LIB_NAME} PRIVATE
        ${COMMON_LIB_NAME}
        )

target_precompile_headers(${COMMON_LIB_NAME} INTERFACE pch.h)

add_subdirectory(assets)