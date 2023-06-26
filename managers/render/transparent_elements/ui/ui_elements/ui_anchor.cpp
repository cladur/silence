#include "ui_anchor.h"
#include <display/display_manager.h>
#include <render/transparent_elements/ui_manager.h>

UIAnchor::UIAnchor(float x, float y) : x(x), y(y) {
	auto &rm = ResourceManager::get();
	alignment = Alignment::NONE;
	size = glm::vec2(1.0f, 1.0f);
	texture = rm.get_texture_handle("anchor_debug");
	glm::vec2 d_size = DisplayManager::get().get_window_size();
	position = glm::vec3(d_size.x * x, d_size.y * y, -5.0f);
}

void UIAnchor::draw() {
	if (!display) {
		for (auto &child : children) {
			child->display = false;
		}
		return;
	}

	auto &ui_manager = UIManager::get();

	glm::vec2 d_size = DisplayManager::get().get_window_size();
	position = glm::vec3(d_size.x * x, d_size.y * y, -1.0f);

	if (draw_anchor) {
		ui_manager.sprite_draw.draw_sprite(
				position, glm::vec2(66.0f, 66.0f), color, texture, is_screen_space, 0.0f, alignment);
	}

	for (auto &child : children) {
		if (child->display == false) {
			continue;
		}
		child->display = display;
		child->parent_position = position;
		child->draw(position, size);
	}
}

void UIAnchor::draw(glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) {
		for (auto &child : children) {
			child->display = false;
		}
		return;
	}

	auto &ui_manager = UIManager::get();

	glm::vec2 d_size = parent_size;

	glm::vec3 new_pos =
			parent_position - glm::vec3(parent_size / 2.0f, 0.0f) + glm::vec3(d_size.x * x, d_size.y * y, 0.01f);

	ui_manager.sprite_draw.draw_sprite(new_pos, size, color, texture, is_screen_space, 0.0f, alignment);

	for (auto &child : children) {
		child->parent_position = new_pos;
		child->draw(new_pos, size);
	}
}
