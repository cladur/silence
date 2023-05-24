set(ENGINE_NAME "silence_engine")

add_library(${ENGINE_NAME} STATIC
        engine.cpp engine.h
        scene.cpp scene.h
        )

# Define the link libraries
target_link_libraries(${ENGINE_NAME} PUBLIC
        silence_common
        silence_core
        silence_display_manager
        silence_ecs_manager
        silence_audio_manager
        silence_scene_manager
        silence_input_manager
        silence_font_manager
        silence_render_manager
        silence_resource_manager
        silence_physics_manager
        silence_animation_manager
        silence_ai_manager
        )