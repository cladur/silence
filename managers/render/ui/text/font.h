#ifndef SILENCE_FONT_H
#define SILENCE_FONT_H

#include "ft2build.h"
#include <render/render_manager.h>
#include FT_FREETYPE_H
#include "render/ui/texture_atlas.h"

//define a Character struct
struct CharacterGlyph {
	// glyph texture coordinates in atlas. x, y are x offsets and z is the y offset down.
	// that os we're assuming the glyph always start on top of the atlas. for now.
	glm::vec3 atlas_pos;
	glm::ivec2 size; // Size of glyph
	glm::ivec2 bearing; // Offset from baseline to left/top of glyph
	unsigned int advance; // Horizontal offset to advance to next glyph
};

class Font {
private:
	FT_Face face;
	std::unordered_map<char, CharacterGlyph> glyph_data;
	TextureAtlas atlas;

public:
	vk::Sampler sampler;

	explicit Font(FT_Face face, TextureAtlas atlas);
	~Font();

	CharacterGlyph *get_character_glyph_data(char c);
};

#endif //SILENCE_FONT_H
