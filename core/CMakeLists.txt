set(CORE_LIB_NAME "silence_core")

# Add source files
file(GLOB_RECURSE SOURCE_FILES
        *.c
        *.cpp)

# Add header files
file(GLOB_RECURSE HEADER_FILES
        *.h
        *.hpp)

add_library(${CORE_LIB_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} input/input_manager.h input/input_manager.cpp input/input_key.h input/input_devices.h input/multiplatform_input.h input/multiplatform_input.cpp types.h components/transform_component.h components/rigidbody_component.h components/gravity_component.h components/parent_component.h components/children_component.h components/state_component.h ai/state_machine/base_state.h ai/state_machine/states/test_state.cpp ai/state_machine/states/test_state.h)

target_include_directories(${CORE_LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Linking libraries

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE magic_enum::magic_enum)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE glm::glm)