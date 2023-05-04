#include "ui_anchor.h"
#include <display/display_manager.h>

UIAnchor::UIAnchor(float x, float y) : x(x), y(y) {
	alignment = sprite_draw::Alignment::NONE;
	size = glm::vec2(1.0f, 1.0f);
	texture_name = "anchor_debug";
	glm::vec2 d_size = DisplayManager::get().get_window_size();
	position = glm::vec3(d_size.x * x, d_size.y * y, -99.0f);
}

void UIAnchor::draw() {
	if (!display) {
		return;
	}
	glm::vec2 d_size = DisplayManager::get().get_window_size();
	position = glm::vec3(d_size.x * x, d_size.y * y, -1.0f);

	if (draw_anchor) {
		sprite_draw::draw_sprite(
				position, glm::vec2(66.0f, 66.0f), color, texture_name.c_str(), is_screen_space, alignment);
	}

	for (auto &child : children) {
		child->draw(position, size);
	}
}

void UIAnchor::draw(glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) {
		return;
	}
	glm::vec2 d_size = parent_size;

	glm::vec3 new_pos =
			parent_position - glm::vec3(parent_size / 2.0f, 0.0f) + glm::vec3(d_size.x * x, d_size.y * y, 0.01f);

	sprite_draw::draw_sprite(new_pos, size, color, texture_name.c_str(), is_screen_space, alignment);

	for (auto &child : children) {
		child->draw(new_pos, size);
	}
}
