set(AUDIO_MANAGER_NAME "silence_audio_manager")

add_library(${AUDIO_MANAGER_NAME} STATIC
        audio_manager.cpp audio_manager.h
        fmod_listener_system.h fmod_listener_system.cpp
        event_reference.cpp event_reference.h
        adaptive_music_manager.cpp adaptive_music_manager.h ecs/fmod_emitter_system.cpp ecs/fmod_emitter_system.h)

target_include_directories(${AUDIO_MANAGER_NAME} PUBLIC ${MANAGER_DIR})

target_link_libraries(${AUDIO_MANAGER_NAME}
        PUBLIC
        fmod
        PRIVATE
        silence_common
        silence_core
        silence_gameplay_manager
        )
