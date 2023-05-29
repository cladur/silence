#include "ui_image.h"
#include "render/transparent_elements/ui_manager.h"

UIImage::UIImage(glm::vec3 position, glm::vec2 size, const Handle<Texture> &texture_handle) {
	this->position = position;
	this->size = size;
	this->texture = texture_handle;
}

void UIImage::draw() {
	if (!display) { return; }

	auto &ui_manager = UIManager::get();

	if (is_billboard)
	{
		ui_manager.sprite_draw.draw_sprite_billboard(position, size, color, texture, use_camera_right);
	} else {
		ui_manager.sprite_draw.draw_sprite(position, size, color, texture, is_screen_space, alignment);
	}

	for (auto& child : children) {
		child->draw(position, size);
	}
}

void UIImage::draw(glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) { return; }
	auto &ui_manager = UIManager::get();
	glm::vec3 new_pos = position + parent_position;
	new_pos.z += 0.01f;

	if (ResourceManager::get().get_texture_name(texture).empty()) {
		ui_manager.sprite_draw.draw_colored(new_pos, size, color, is_screen_space, alignment);
	} else {
		ui_manager.sprite_draw.draw_sprite(new_pos, size, color, texture, is_screen_space, Alignment::NONE);
	}

	for (auto& child : children) {
		child->draw(new_pos, size);
	}
}
