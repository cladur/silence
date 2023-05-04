add_subdirectory(fmod)

add_subdirectory(nativefiledialog-extended)

set(ImGuizmo_SOURCE_DIR ImGuizmo)
add_library(imguizmo STATIC ${ImGuizmo_SOURCE_DIR}/ImGuizmo.cpp
                            ${ImGuizmo_SOURCE_DIR}/GraphEditor.cpp
                            ${ImGuizmo_SOURCE_DIR}/ImCurveEdit.cpp
                            ${ImGuizmo_SOURCE_DIR}/ImGradient.cpp
                            ${ImGuizmo_SOURCE_DIR}/ImGuizmo.cpp
                            ${ImGuizmo_SOURCE_DIR}/ImSequencer.cpp)

target_include_directories(imguizmo PUBLIC ${ImGuizmo_SOURCE_DIR})

find_package(imgui CONFIG REQUIRED)

target_link_libraries(imguizmo PRIVATE imgui::imgui)