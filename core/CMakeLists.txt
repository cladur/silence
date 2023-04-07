set(COMMON_LIB_NAME "silence_common")
set(CORE_LIB_NAME "silence_core")

add_library(${COMMON_LIB_NAME} INTERFACE
        pch.h
        types.h)

add_library(${CORE_LIB_NAME} STATIC
        components/transform_component.h
        components/rigidbody_component.h
        components/gravity_component.h
        components/parent_component.h
        components/children_component.h
        components/fmod_listener_component.h
        state_machine/states/state.h
        state_machine/states/test_state.cpp state_machine/states/test_state.h
        state_machine/state_machine.cpp state_machine/state_machine.h
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
        camera/camera.cpp camera/camera.h
        cvars/cvars.cpp cvars/cvars.h
        )

target_include_directories(${CORE_LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Linking libraries
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

target_link_libraries(${COMMON_LIB_NAME} INTERFACE
        spdlog::spdlog spdlog::spdlog_header_only
        imgui::imgui
        glm::glm
        magic_enum::magic_enum
        )

target_link_libraries(${CORE_LIB_NAME} PRIVATE
        ${COMMON_LIB_NAME}
        )

target_precompile_headers(${COMMON_LIB_NAME} INTERFACE pch.h)

add_subdirectory(assets)