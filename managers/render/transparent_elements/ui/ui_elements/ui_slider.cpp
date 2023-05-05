#include "ui_slider.h"
#include "render/transparent_elements/ui/sprite_draw.h"
#include <render/render_scene.h>

UISlider::UISlider(float value, float min, float max)
	: value(value), min(min), max(max) {
}

UISlider::~UISlider() {
}

void UISlider::draw(RenderScene *scene) {
	if (!display) { return; }
	auto &sprite_draw = scene->sprite_draw;
	value = std::clamp(value, min, max);
	if (!is_billboard) {
		glm::vec2 new_size = size;
		glm::vec3 new_position = position;
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

		sprite_draw.draw_colored(new_position + glm::vec3(0.0f, 0.0f, 0.01f), new_size, color, is_screen_space, alignment);
		sprite_draw.draw_colored(position, size, glm::vec3(0.0f), is_screen_space, alignment);
	} else {
		sprite_draw.draw_slider_billboard(
				position,
				0.0f,
				size,
				color,
				(value - min) / (max - min),
				slider_alignment);
		sprite_draw.draw_slider_billboard(
				position,
				-0.01f,
				size,
				glm::vec3(0.0f),
				1.0f,
				slider_alignment);
	}

	for (auto& child : children) {
		child->draw(scene, position, size);
	}
}

void UISlider::draw(RenderScene *scene, glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) { return; }
	auto &sprite_draw = scene->sprite_draw;
	value = std::clamp(value, min, max);
	glm::vec2 new_size = size;

	glm::vec3 combined_position = position + parent_position;

	glm::vec3 new_position = position + parent_position;
	new_position.z += 0.02f;

	switch (slider_alignment) {
		case SliderAlignment::LEFT_TO_RIGHT:
			// offset the position to properly slide the slider
			new_size.x = (value - min) / (max - min) * size.x;
			new_position.x = combined_position.x - (size.x - new_size.x) / 2.0f;
			break;
		case SliderAlignment::RIGHT_TO_LEFT:
			new_size.x = (value - min) / (max - min) * size.x;
			new_position.x = combined_position.x + (size.x - new_size.x) / 2.0f;
			break;
		case SliderAlignment::TOP_TO_BOTTOM:
			new_size.y = (value - min) / (max - min) * size.y;
			new_position.y = combined_position.y + (size.y - new_size.y) / 2.0f;
			break;
		case SliderAlignment::BOTTOM_TO_TOP:
			new_size.y = (value - min) / (max - min) * size.y;
			new_position.y = combined_position.y - (size.y - new_size.y) / 2.0f;
			break;
	}

	// alignment is none since we're drawing relatively to the parent location which is already added.
	// THE WHITE BAR
	sprite_draw.draw_colored(
			new_position,
			new_size,
			color,
			is_screen_space,
			Alignment::NONE);
	// THE BACKGROUND BAR
	sprite_draw.draw_colored(
			position + parent_position + glm::vec3(0.0f, 0.0f, 0.01f),
			size,
			glm::vec3(0.0f),
			is_screen_space,
			Alignment::NONE);

	for (auto& child : children) {
		child->draw(scene, position + parent_position + glm::vec3(0.0f, 0.0f, 0.01f), size);
	}
}
