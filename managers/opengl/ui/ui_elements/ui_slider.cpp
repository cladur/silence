#include "ui_slider.h"

UISlider::UISlider(float value, float min, float max, bool is_stepped)
	: value(value), min(min), max(max), is_stepped(is_stepped) {
}

UISlider::~UISlider() {
}

void UISlider::draw() {
	value = std::clamp(value, min, max);
	glm::vec2 new_size = size;
	glm::vec2 new_position = position;
	switch (slider_alignment) {
		case SliderAlignment::LEFT_TO_RIGHT:
			// offset the position to properly slide the slider
			new_size.x = (value - min) / (max - min) * size.x;
			new_position.x = position.x - (size.x - new_size.x) / 2.0f;
			break;
		case SliderAlignment::RIGHT_TO_LEFT:
			new_size.x = (value - min) / (max - min) * size.x;
			new_position.x = position.x + (size.x - new_size.x) / 2.0f;
			break;
		case SliderAlignment::TOP_TO_BOTTOM:
			new_size.y = (value - min) / (max - min) * size.y;
			new_position.y = position.y + (size.y - new_size.y) / 2.0f;
			break;
		case SliderAlignment::BOTTOM_TO_TOP:
			new_size.y = (value - min) / (max - min) * size.y;
			new_position.y = position.y - (size.y - new_size.y) / 2.0f;
			break;
	}

	sprite_draw::draw_colored(new_position, new_size, color, is_screen_space, alignment);
	sprite_draw::draw_colored(position, size, glm::vec3(0.0f), is_screen_space, alignment);
}
