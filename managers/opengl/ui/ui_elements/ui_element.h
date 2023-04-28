#ifndef SILENCE_UI_ELEMENT_H
#define SILENCE_UI_ELEMENT_H

#include <opengl/ui/sprite_draw.h>
class UIElement {
public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec2 size = glm::vec2(1.0f);
	glm::vec3 color = glm::vec3(1.0f);
	std::string texture_name;
	bool is_screen_space = true;
	bool is_billboard = false;
	sprite_draw::Alignment alignment = sprite_draw::Alignment::NONE;
	std::vector<UIElement*> children;

	virtual void draw();
	virtual void draw(glm::vec3 parent_position, glm::vec2 parent_size);
	virtual void add_child(UIElement &child);
};

#endif //SILENCE_UI_ELEMENT_H
