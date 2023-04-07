#ifndef SILENCE_UI_TEXT_COMPONENT_H
#define SILENCE_UI_TEXT_COMPONENT_H

#include <render/ui/text/font.h>
#include <glm/vec3.hpp>
#include <string>

struct UIText {
	int font_id;
	std::string text;
	glm::vec3 color;
	float scale;
};

#endif //SILENCE_UI_TEXT_COMPONENT_H
