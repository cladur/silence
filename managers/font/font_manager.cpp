#include "font_manager.h"
#include <freetype/freetype.h>

#include "glad/glad.h"

FontManager *FontManager::get() {
	static FontManager instance;
	return &instance;
}

void FontManager::startup() {
	if (FT_Init_FreeType(&ft)) {
		SPDLOG_ERROR("FREETYPE: Could not init FreeType Library");
	}
}

void FontManager::shutdown() {
	FT_Done_FreeType(ft);
}

void FontManager::load_font(const char *path, int size) {
	FT_Face face;
	if (FT_New_Face(ft, path, 0, &face)) {
		SPDLOG_ERROR("FREETYPE: Failed to load font");
	}

	FT_Set_Pixel_Sizes(face, 0, size);

	const int num_glyphs = 128;

	// quick and dirty max texture size estimate
	int max_dim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(num_glyphs));
	int tex_width = 1;
	while (tex_width < max_dim) {
		tex_width <<= 1;
	}
	int tex_height = tex_width;

	// render glyphs to atlas

	Font font = {};
	font.texture_size = glm::vec2(tex_width, tex_height);

	int texture_size = tex_width * tex_height;
	char *pixels = (char *)calloc(texture_size, 1);
	FT_Pos pen_x = 0, pen_y = 0;

	for (int i = 0; i < num_glyphs; ++i) {
		FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
		FT_Bitmap *bmp = &face->glyph->bitmap;

		// check if we need to move to the next row
		if (pen_x + bmp->width >= tex_width) {
			pen_x = 0;
			pen_y += ((face->size->metrics.height >> 6) + 1);
		}

		// copy glyph to atlas
		for (int row = 0; row < bmp->rows; ++row) {
			for (int col = 0; col < bmp->width; ++col) {
				int x = (int)pen_x + col;
				int y = (int)pen_y + row;
				pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
			}
		}

		// add character to map
		Character ch = {};
		ch.x_min = pen_x;
		ch.y_min = pen_y;
		ch.x_max = pen_x + bmp->width;
		ch.y_max = pen_y + bmp->rows;
		ch.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
		ch.advance = face->glyph->advance.x;

		font.characters.insert(std::pair<char, Character>(i, ch));

		// add 1 pixel of padding between glyphs
		pen_x += bmp->width + 1;
	}

	glGenTextures(1, &font.texture);
	glBindTexture(GL_TEXTURE_2D, font.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	free(pixels);
	FT_Done_Face(face);

	fonts.insert(std::pair<std::string, Font>(path, font));
}