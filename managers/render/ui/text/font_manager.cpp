#include "font_manager.h"
#include <filesystem>
#include <iostream>
#include <map>
#include <string>

void FontManager::startup() {
	auto er = FT_Init_FreeType(&ft);
	if (er) {
		SPDLOG_ERROR("Could not init FreeType Library");
	}
	SPDLOG_INFO("Initialized Font Manager");
}

void FontManager::shutdown() {
	FT_Done_FreeType(ft);
}

void FontManager::load_font(const char *path, int size) {
	FT_Face face;
	// initialize face

	SPDLOG_INFO("Loading font: {}", path);
	auto error = FT_New_Face(ft, path, 0, &face);
	if (error) {
		SPDLOG_ERROR("Failed to load font: {}", path);
	}

	FT_Set_Pixel_Sizes(face, 0, size);
	TextureAtlas atlas = TextureAtlas(glm::ivec2(4096, 128));
	Font f = Font(face, atlas);
	fonts.push_back(f);
}

Font &FontManager::get_font(int id) {
	if (id >= fonts.size()) {
		SPDLOG_ERROR("Font id {} is out of range, returning font of id 0", id);
		return fonts[0];
	}
	return fonts[id];
}

