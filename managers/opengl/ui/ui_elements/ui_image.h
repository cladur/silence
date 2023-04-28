#ifndef SILENCE_UI_IMAGE_H
#define SILENCE_UI_IMAGE_H

#include "ui_element.h"

class UIImage : public UIElement {
public:
	UIImage(glm::vec2 position, glm::vec2 size, const std::string& texture_name);

	void draw() override;
	void draw(glm::vec3 parent_position, glm::vec2 parent_size) override;
};

#endif //SILENCE_UI_IMAGE_H
