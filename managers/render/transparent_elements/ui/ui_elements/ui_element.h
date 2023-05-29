#ifndef SILENCE_UI_ELEMENT_H
#define SILENCE_UI_ELEMENT_H

#include "render/transparent_elements/ui/sprite_draw.h"
#include <render/common/texture.h>
#include <resource/resource_manager.h>

struct RenderScene;

class UIElement {
public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec2 size = glm::vec2(1.0f);
	glm::vec4 color = glm::vec4(1.0f);
	Handle<Texture> texture;
	std::string font_name;
	bool is_screen_space = true;
	bool is_billboard = false;
	std::vector<UIElement *> children;
	Alignment alignment = Alignment::NONE;
	bool display = true;

	glm::vec3 parent_position = glm::vec3(0.0f);

	UIElement() = default;

	virtual void draw();
	virtual void draw(glm::vec3 parent_position, glm::vec2 parent_size);
	virtual void add_child(UIElement &child);
};

#endif //SILENCE_UI_ELEMENT_H
