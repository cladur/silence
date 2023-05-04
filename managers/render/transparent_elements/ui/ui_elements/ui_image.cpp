#include "ui_image.h"
#include <render/render_scene.h>

UIImage::UIImage(glm::vec3 position, glm::vec2 size, const std::string& texture_name) {
	this->position = position;
	this->size = size;
	this->texture_name = texture_name;
}

void UIImage::draw(RenderScene *scene) {
	if (!display) { return; }

	auto &sprite_draw = scene->sprite_draw;

	if (is_billboard)
	{
		sprite_draw.draw_sprite_billboard(position, size, color, texture_name.c_str());
	} else {
		sprite_draw.draw_sprite(position, size, color, texture_name.c_str(), is_screen_space, alignment);
	}

	for (auto& child : children) {
		child->draw(scene, position, size);
	}
}

void UIImage::draw(RenderScene *scene, glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) { return; }
	auto &sprite_draw = scene->sprite_draw;
	glm::vec3 new_pos = position + parent_position;
	new_pos.z += 0.01f;

	if (texture_name.empty()) {
		sprite_draw.draw_colored(new_pos, size, color, is_screen_space, alignment);
	} else {
		sprite_draw.draw_sprite(new_pos, size, color, texture_name.c_str(), is_screen_space, Alignment::NONE);
	}

	for (auto& child : children) {
		child->draw(scene, new_pos, size);
	}
}
