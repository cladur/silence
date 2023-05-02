#include "ui_image.h"

UIImage::UIImage(glm::vec3 position, glm::vec2 size, const std::string& texture_name) {
	this->position = position;
	this->size = size;
	this->texture_name = texture_name;
}

void UIImage::draw() {
	if (is_billboard)
	{
		sprite_draw::draw_sprite_billboard(position, size, color, texture_name.c_str());
	} else {
		sprite_draw::draw_sprite(position, size, color, texture_name.c_str(), is_screen_space, alignment);
	}

	for (auto& child : children) {
		child->draw(position, size);
	}
}

void UIImage::draw(glm::vec3 parent_position, glm::vec2 parent_size) {
	glm::vec3 new_pos = position + parent_position;
	new_pos.z += 0.01f;

	sprite_draw::draw_sprite(new_pos, size, color, texture_name.c_str(), is_screen_space, sprite_draw::Alignment::NONE);

	for (auto& child : children) {
		child->draw(new_pos, size);
	}
}
