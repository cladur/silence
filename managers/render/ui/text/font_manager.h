#ifndef SILENCE_FONT_MANAGER_H
#define SILENCE_FONT_MANAGER_H

#include "font.h"
#include "ft2build.h"
#include FT_FREETYPE_H

class FontManager {
private:
	FT_Library ft;
	std::vector<Font> fonts;

public:
	void startup();
	void shutdown();

	// loads a font with a given pixel size
	void load_font(const char *path, int size);
	Font &get_font(int id);

};

#endif //SILENCE_FONT_MANAGER_H
