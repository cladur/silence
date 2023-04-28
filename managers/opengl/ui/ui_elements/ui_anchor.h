#ifndef SILENCE_UI_ANCHOR_H
#define SILENCE_UI_ANCHOR_H

#include "ui_element.h"

class UIAnchor : public UIElement {
public:
	float x = 0.0f;
	float y = 0.0f;

	UIAnchor(float x, float y);

	void draw() override;
	void draw(glm::vec3 parent_position, glm::vec2 parent_size) override;
};

#endif //SILENCE_UI_ANCHOR_H
