set(ASSET_BAKER_NAME "silence_baker")

add_executable(${ASSET_BAKER_NAME}
        main.cpp
        )

# Define the link libraries
target_link_libraries(${ASSET_BAKER_NAME} PRIVATE
        silence_common
        silence_assets
        )