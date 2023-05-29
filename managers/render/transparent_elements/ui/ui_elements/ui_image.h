#ifndef SILENCE_UI_IMAGE_H
#define SILENCE_UI_IMAGE_H

#include "ui_element.h"

class UIImage : public UIElement {
public:
	UIImage() = default;
	UIImage(glm::vec3 position, glm::vec2 size, const Handle<Texture> &texture_handle);

	void draw() override;
	void draw(glm::vec3 parent_position, glm::vec2 parent_size) override;
};

#endif //SILENCE_UI_IMAGE_H
