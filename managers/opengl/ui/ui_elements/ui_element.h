#ifndef SILENCE_UI_ELEMENT_H
#define SILENCE_UI_ELEMENT_H

#include <opengl/ui/sprite_draw.h>
class UIElement {
public:
	glm::vec2 position = glm::vec2(0.0f);
	glm::vec2 size = glm::vec2(1.0f);
	glm::vec4 color = glm::vec4(1.0f);
	std::string texture_name;
	bool is_screen_space = true;
	sprite_draw::Alignment alignment = sprite_draw::Alignment::CENTER;

	virtual void draw() = 0;
};

#endif //SILENCE_UI_ELEMENT_H
