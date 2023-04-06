#ifndef SILENCE_FONT_MANAGER_H
#define SILENCE_FONT_MANAGER_H

#include "ft2build.h"
#include FT_FREETYPE_H

class FontManager {
private:
	FT_Library ft;

public:
	FontManager();
	~FontManager();

	// loads a font with a given pixel size
	FT_Face load_font(const char *path, int size);
};

#endif //SILENCE_FONT_MANAGER_H
