set(EDITOR_NAME "silence_editor")

add_executable(${EDITOR_NAME}
        editor.cpp editor.h
        editor_scene.cpp editor_scene.h
        gui.cpp
        main.cpp
        inspector_gui.cpp inspector_gui.h
        )

find_package(OpenMP)

# Define the link libraries
target_link_libraries(${EDITOR_NAME} PRIVATE
        silence_engine
        nfd
        imguizmo
        OpenMP::OpenMP_CXX
        )