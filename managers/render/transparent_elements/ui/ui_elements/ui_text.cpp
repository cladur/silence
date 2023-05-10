#include "ui_text.h"
#include "render/transparent_elements/ui_manager.h"
#include <render/render_scene.h>

UIText::UIText(glm::vec3 position, float scale, const std::string &text, const std::string &font_name) {
	this->position = position;
	this->size = glm::vec2(scale, scale);
	this->text = text;
	this->texture_name = font_name;
}

void UIText::draw() {
	if (!display) {
		return;
	}
	if (text.empty()) {
		return;
	}
	UIManager::get().text_draw.draw_text(text, is_screen_space, position, color, size.x, texture_name, centered_x, centered_y,
	 		glm::vec3(0.0f), is_billboard);

	for (auto &child : children) {
		child->draw(position, size);
	}
}

void UIText::draw(glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) {
		return;
	}
	if (text.empty()) {
		return;
	}
	UIManager::get().text_draw.draw_text(text, is_screen_space, position + parent_position, color, size.x, texture_name, centered_x,
	 		centered_y, glm::vec3(0.0f), is_billboard);

	for (auto &child : children) {
		child->draw(position, size);
	}
}
