set(FMOD_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api/core)
set(FMOD_FSBANK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api/fsbank)
set(FMOD_STUDIO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api/studio)

set(FMOD_INCLUDE_DIRS
        ${FMOD_CORE_DIR}/inc
        ${FMOD_FSBANK_DIR}/inc
        ${FMOD_STUDIO_DIR}/inc
        CACHE INTERNAL "FMOD include directories"
        )

find_library(FMOD_CORE_LIB
        NAMES fmod fmod_vc
        PATHS
        ${FMOD_CORE_DIR}/lib
        ${FMOD_CORE_DIR}/lib/x64
        )

find_library(FMOD_FSBANK_LIB
        NAMES fsbank fsbank_vc
        PATHS
        ${FMOD_FSBANK_DIR}/lib
        ${FMOD_FSBANK_DIR}/lib/x64
        )

find_library(FMOD_STUDIO_LIB
        NAMES fmodstudio fmodstudio_vc
        PATHS
        ${FMOD_STUDIO_DIR}/lib
        ${FMOD_STUDIO_DIR}/lib/x64
        )

set(FMOD_LIBRARIES
        ${FMOD_CORE_LIB}
        ${FMOD_FSBANK_LIB}
        ${FMOD_STUDIO_LIB}
        CACHE INTERNAL "FMOD libraries"
        )

# Windows needs DLLs at runtime
IF(WIN32)
    file(
            COPY
            ${FMOD_CORE_DIR}/lib/x64/fmod.dll
            ${FMOD_FSBANK_DIR}/lib/x64/fsbank.dll
            ${FMOD_FSBANK_DIR}/lib/x64/libfsbvorbis64.dll
            ${FMOD_FSBANK_DIR}/lib/x64/opus.dll
            ${FMOD_STUDIO_DIR}/lib/x64/fmodstudio.dll
            DESTINATION
            ${CMAKE_BINARY_DIR}
    )
ENDIF()

add_library(fmod INTERFACE)
target_link_libraries(fmod INTERFACE ${FMOD_LIBRARIES})
target_include_directories(fmod INTERFACE ${FMOD_INCLUDE_DIRS})