#ifndef SILENCE_UI_TEXT_COMPONENT_H
#define SILENCE_UI_TEXT_COMPONENT_H

#include <glm/vec3.hpp>
#include <string>

struct UIText {
	std::string text;
	glm::vec3 color;
	float scale;
};

#endif //SILENCE_UI_TEXT_COMPONENT_H
