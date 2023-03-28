set(CORE_LIB_NAME "silence_core")

# Add source files
file(GLOB_RECURSE SOURCE_FILES
        *.c
        *.cpp)

# Add header files
file(GLOB_RECURSE HEADER_FILES
        *.h
        *.hpp)

add_library(${CORE_LIB_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} input/Input.cpp input/input.h input/input.cpp input/input.h types.h components/transform_component.h components/rigidbody_component.h components/gravity_component.h state_machine/state.h state_machine/states/test_state.cpp state_machine/states/test_state.h state_machine/state_machine.cpp state_machine/state_machine.h)

target_include_directories(${CORE_LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Linking libraries

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE glm::glm)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)