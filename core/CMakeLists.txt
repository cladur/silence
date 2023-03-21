set(CORE_LIB_NAME "silence_core")

# Add source files
file(GLOB_RECURSE SOURCE_FILES
        *.c
        *.cpp)

# Add header files
file(GLOB_RECURSE HEADER_FILES
        *.h
        *.hpp)

add_library(${CORE_LIB_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} input/Input.cpp input/input.h input/input.cpp input/input.h)

target_include_directories(${CORE_LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Linking libraries

target_link_libraries(${CORE_LIB_NAME} PRIVATE silence_common)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE glfw)