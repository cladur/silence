#ifndef SILENCE_FONT_H
#define SILENCE_FONT_H

#include "ft2build.h"
#include <render/render_manager.h>
#include FT_FREETYPE_H
#include "render/ui/texture_atlas.h"

//define a Character struct
struct CharacterGlyph {
	int atlas_pos; // glyph texture
	glm::ivec2 size; // Size of glyph
	glm::ivec2 bearing; // Offset from baseline to left/top of glyph
	glm::ivec2 advance; // Horizontal offset to advance to next glyph
};

class Font {
private:
	FT_Face face;
	std::unordered_map<char, CharacterGlyph> atlas_positions;
	TextureAtlas atlas;

public:
	vk::Sampler sampler;

	explicit Font(FT_Face face, TextureAtlas atlas);
	~Font();

	CharacterGlyph *get_character_glyph(char c);
};

#endif //SILENCE_FONT_H
