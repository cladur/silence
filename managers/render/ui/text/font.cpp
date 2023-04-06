#include "font.h"

extern RenderManager render_manager;

Font::Font(FT_Face face) {
	this->face = face;

	// convert face to textures and store them in the characters map
	for (unsigned char c = 33; c < 128; c++) {

		auto error = FT_Load_Char(face, (char)c, FT_LOAD_DEFAULT);
		if(error) {
			throw std::runtime_error("failed to load glyph");
		}

		FT_GlyphSlot glyphSlot = face->glyph;
		error = FT_Render_Glyph(glyphSlot, FT_RENDER_MODE_NORMAL);
		if(error) {
			throw std::runtime_error("failed to render glyph");
		}

		SPDLOG_INFO("Loading Glyph: {}", (char)c);

		characters.insert(std::pair<char, CharacterGlyph>(c, CharacterGlyph{
			render_manager.get_character_texture(face),
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x)
		}));

		SPDLOG_INFO("Glyph {} Loaded", (char)c);
	}

	FT_Done_Face(face);
}

Font::~Font() {
}

CharacterGlyph *Font::get_character_glyph(char c) {
	return &characters[c];
}
