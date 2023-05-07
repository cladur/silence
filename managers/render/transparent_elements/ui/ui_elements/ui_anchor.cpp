#include "ui_anchor.h"
#include <display/display_manager.h>
#include <render/render_scene.h>

UIAnchor::UIAnchor(float x, float y) : x(x), y(y) {
	alignment = Alignment::NONE;
	size = glm::vec2(1.0f, 1.0f);
	texture_name = "anchor_debug";
	glm::vec2 d_size = DisplayManager::get().get_window_size();
	position = glm::vec3(d_size.x * x, d_size.y * y, -5.0f);
}

void UIAnchor::draw(RenderScene *scene) {
	if (!display) {
		for (auto &child : children) {
			child->display = false;
		}
		return;
	}

	glm::vec2 d_size = DisplayManager::get().get_window_size();
	position = glm::vec3(d_size.x * x, d_size.y * y, -1.0f);

	if (draw_anchor) {
		scene->sprite_draw.draw_sprite(
				position, glm::vec2(66.0f, 66.0f), color, texture_name.c_str(), is_screen_space, alignment);
	}

	for (auto &child : children) {
		child->display = display;
		child->parent_position = position;
		child->draw(scene, position, size);
	}
}

void UIAnchor::draw(RenderScene *scene, glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) {
		for (auto &child : children) {
			child->display = false;
		}
		return;
	}

	auto &sprite_draw = scene->sprite_draw;
	glm::vec2 d_size = parent_size;

	glm::vec3 new_pos =
			parent_position - glm::vec3(parent_size / 2.0f, 0.0f) + glm::vec3(d_size.x * x, d_size.y * y, 0.01f);

	sprite_draw.draw_sprite(new_pos, size, color, texture_name.c_str(), is_screen_space, alignment);

	for (auto &child : children) {
		child->parent_position = new_pos;
		child->draw(scene, new_pos, size);
	}
}
