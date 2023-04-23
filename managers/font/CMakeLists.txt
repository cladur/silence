set(FONT_MANAGER_NAME "silence_font_manager")

# Libraries
add_library(${FONT_MANAGER_NAME} STATIC
        font_manager.cpp font_manager.h
        )

# Include directories
target_include_directories(${FONT_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

# Link libraries
find_package(Freetype REQUIRED)

target_link_libraries(${FONT_MANAGER_NAME} PRIVATE
        silence_common
        silence_core
        Freetype::Freetype
        )