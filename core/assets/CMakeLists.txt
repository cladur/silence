set(ASSETS_LIB_NAME "silence_assets")

add_library(${ASSETS_LIB_NAME} STATIC
        asset_loader.cpp asset_loader.h
        texture_asset.cpp texture_asset.h
        mesh_asset.cpp mesh_asset.h material_asset.cpp material_asset.h prefab_asset.cpp prefab_asset.h)

target_include_directories(${ASSETS_LIB_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Linking libraries
find_package(nlohmann_json CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)

target_link_libraries(${ASSETS_LIB_NAME} PRIVATE
        silence_common
        nlohmann_json::nlohmann_json
        lz4::lz4
        KTX::ktx
        )