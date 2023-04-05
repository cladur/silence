#include "asset_loader.h"
#include "mesh_asset.h"
#include "texture_asset.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fs = std::filesystem;
using namespace assets;

bool convert_image(const fs::path &input, const fs::path &output) {
	SPDLOG_INFO("Converting image {} to {}", input.string(), output.string());

	int tex_width, tex_height, tex_channels;

	stbi_uc *pixels =
			stbi_load((const char *)input.u8string().c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

	if (!pixels) {
		SPDLOG_ERROR("Failed to load texture file {}", input.string());
		return false;
	}

	int texture_size = tex_width * tex_height * 4;

	TextureInfo tex_info;
	tex_info.texture_size = texture_size;
	tex_info.pixel_size[0] = tex_width;
	tex_info.pixel_size[1] = tex_height;
	tex_info.texture_format = TextureFormat::RGBA8;
	tex_info.original_file = input.string();
	assets::AssetFile new_image = assets::pack_texture(&tex_info, pixels);

	stbi_image_free(pixels);

	save_binary_file(output.string().c_str(), new_image);

	return true;
}

bool convert_mesh(const fs::path &input, const fs::path &output) {
	SPDLOG_INFO("Converting mesh {} to {}", input.string(), output.string());

	return true;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		SPDLOG_ERROR("You need to put a asset directory as an argument");
		return 1;
	}

	fs::path path{ argv[1] };

	const fs::path &directory = path;

	SPDLOG_INFO("Loading asset directory at {}", directory.string());

	for (auto &p : fs::directory_iterator(directory)) {
		SPDLOG_INFO("File: {}", p.path().string());

		if (p.path().extension() == ".png") {
			SPDLOG_INFO("found a texture");

			auto new_path = p.path();
			new_path.replace_extension(".tx");
			convert_image(p.path(), new_path);
		}
		if (p.path().extension() == ".obj") {
			SPDLOG_INFO("found a mesh");

			auto new_path = p.path();
			new_path.replace_extension(".mesh");
			convert_mesh(p.path(), new_path);
		}
	}
}