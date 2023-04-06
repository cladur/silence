#ifndef SILENCE_FONT_H
#define SILENCE_FONT_H

#include "ft2build.h"
#include <render/render_manager.h>
#include FT_FREETYPE_H

//define a Character struct
struct CharacterGlyph {
	Texture texture; // glyph texture
	glm::ivec2 size; // Size of glyph
	glm::ivec2 bearing; // Offset from baseline to left/top of glyph
	unsigned int advance; // Horizontal offset to advance to next glyph
};

class Font {
private:
	FT_Face face;
	std::unordered_map<char, CharacterGlyph> characters;
public:
	explicit Font(FT_Face face);
	~Font();

	CharacterGlyph *get_character_glyph(char c);
};

#endif //SILENCE_FONT_H
