#ifndef SILENCE_FONT_MANAGER_H
#define SILENCE_FONT_MANAGER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include "render/common/texture.h"

struct Character {
	int x_min, y_min, x_max, y_max; // Coords of glyph in the texture atlas
	glm::ivec2 bearing; // Offset from baseline to left/top of glyph
	unsigned int advance; // Offset to advance to next glyph
};

struct Font {
	glm::vec2 texture_size;
	Texture texture;
	std::map<char, Character> characters;
};

class FontManager {
	FT_Library ft;

public:
	std::map<std::string, Font> fonts;

	static FontManager &get();

	void startup();
	void shutdown();

	void load_font(const char *path, int size, std::string name);
};

#endif // SILENCE_FONT_MANAGER_H