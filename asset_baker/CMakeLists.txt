set(ASSET_BAKER_NAME "silence_baker")

add_executable(${ASSET_BAKER_NAME}
        main.cpp
        opengl_context.cpp opengl_context.h
        )

# Define the link libraries
target_link_libraries(${ASSET_BAKER_NAME} PRIVATE
        silence_common
        silence_core
        silence_assets
        silence_font_manager
        silence_display_manager
        silence_opengl_manager
        glfw
        glad::glad
        )