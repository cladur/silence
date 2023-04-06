#include "font_manager.h"

#include <iostream>
#include <map>
#include <string>

FontManager::FontManager() {
	if (FT_Init_FreeType(&ft)) {
		SPDLOG_ERROR("Could not init FreeType Library");
	}
}

FontManager::~FontManager() {
	FT_Done_FreeType(ft);
}

FT_Face FontManager::load_font(const char *path, int size) {

	FT_Face face;
	SPDLOG_INFO("Loading font: {}", path);
	if (FT_New_Face(ft, path, 0, &face)) {
		SPDLOG_ERROR("Failed to load font: {}", path);
		return nullptr;
	}

	FT_Set_Pixel_Sizes(face, 0, size);
	return face;
}
