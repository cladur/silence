#include "font.h"

extern RenderManager render_manager;

Font::Font(FT_Face face, TextureAtlas atlas) : atlas(atlas){
	this->face = face;

	// convert face to textures and store them in the characters map
	// todo: i cant load more than one texture at a time so this loop is a single iteration.
	// should be from 33 to 126
	for (unsigned char c = 33; c < 127; c++) {

		auto error = FT_Load_Char(face, (char)c, FT_LOAD_DEFAULT);
		if(error) {
			throw std::runtime_error("failed to load glyph");
		}

		FT_GlyphSlot glyphSlot = face->glyph;
		error = FT_Render_Glyph(glyphSlot, FT_RENDER_MODE_NORMAL);
		if(error) {
			throw std::runtime_error("failed to render glyph");
		}

		if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
			SPDLOG_INFO("Glyph {} is empty", (char)c);
			continue;
		}

		int width = face->glyph->bitmap.width;
		int height = face->glyph->bitmap.rows;

		std::vector<uint8_t> rgba(width * height * 4);
		int idx = 0;
		int idx2 = 0;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				uint8_t gray = face->glyph->bitmap.buffer[idx2++];
				rgba[idx++] = gray;
				rgba[idx++] = gray;
				rgba[idx++] = gray;
				rgba[idx++] = gray;
			}
		}

		void *rgba_ptr = rgba.data();

		glm::vec3 char_pos = atlas.add(rgba_ptr, glm::ivec2(width, height));
		CharacterGlyph char_glyph = {
					char_pos,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x >> 6),
		};

		glyph_data.insert(std::pair<char, CharacterGlyph>(c, char_glyph));

//		characters.insert(std::pair<char, CharacterGlyph>(c, CharacterGlyph{
//			render_manager.get_character_texture(face),
//			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//			glm::ivec2(face->glyph->advance.x >> 6, face->glyph->advance.y >> 6),
//		}));
		SPDLOG_INFO("Loading Glyph: {}, size {}x{}, advance {}, atlas x1 and x2 {}, {}", (char)c, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->advance.x >> 6, char_pos.x, char_pos.y);
	}

	sampler = render_manager.create_sampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToBorder);
	render_manager.glyph_sampler = sampler;
	FT_Done_Face(face);
}

Font::~Font() {
}

CharacterGlyph *Font::get_character_glyph_data(char c) {
	return &glyph_data[c];
}
