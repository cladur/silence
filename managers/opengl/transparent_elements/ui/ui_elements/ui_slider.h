#ifndef SILENCE_UI_SLIDER_H
#define SILENCE_UI_SLIDER_H

#include "ui_element.h"

class UISlider : public UIElement {
public:
	float value = 0.0f;
	float min = 0.0f;
	float max = 1.0f;
	bool is_stepped = false;
	sprite_draw::SliderAlignment slider_alignment = sprite_draw::SliderAlignment::LEFT_TO_RIGHT;

	UISlider() = default;
	UISlider(float value, float min, float max);
	~UISlider();

	void draw() override;
	void draw(glm::vec3 parent_position, glm::vec2 parent_size) override;
};

#endif //SILENCE_UI_SLIDER_H
