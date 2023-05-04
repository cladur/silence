#include "ui_button.h"
#include <audio/audio_manager.h>
#include <display/display_manager.h>
#include <input/input_manager.h>
#include <render/transparent_elements/text/text_draw.h>
#include <render/transparent_elements/ui/sprite_manager.h>

extern InputManager input_manager;
extern AudioManager audio_manager;

UIButton::UIButton(glm::vec3 position, glm::vec2 size, const std::string &text, const std::string &font_name,
		const std::string &texture_name, const std::string &hover_sound_name, const std::string &click_sound_name) {
	this->position = position;
	this->size = size;
	this->text = text;
	this->font_name = font_name;
	this->texture_name = texture_name;
	this->is_screen_space = true;
	color = glm::vec4(1.0f);

	// TODO: in the future remove magic text and put the passed values in here
	hover_event = EventReference("SFX/UI/button_hover");
	click_event = EventReference("SFX/UI/button_click");
	//	if (!hover_sound_name.empty()) {
	//
	//	}
	//	if (!click_sound_name.empty()) {
	//
	//	}
}

void UIButton::draw() {
	if (!display) {
		return;
	}
	std::string tex = texture_name;
	bool h = hovered();

	if (h) {
		if (!hover_texture_name.empty()) {
			tex = hover_texture_name;
		}

		if (!hover_event.path.empty() && !is_hovered) {
			audio_manager.play_one_shot_2d(hover_event);
			is_hovered = true;
		}
	}

	if (texture_name.empty()) {
		sprite_draw::draw_colored(position, size, color, is_screen_space);
	} else {
		sprite_draw::draw_sprite(position, size, color, tex.c_str(), is_screen_space);
	}

	if (text.empty()) {
		return;
	}
	// text_draw::draw_text(text, is_screen_space, position + glm::vec3(0.0f, 0.0f, 0.1f), text_color, text_scale,
	// 		font_name, centered_x, centered_y, glm::vec3(0.0f));
}

void UIButton::draw(glm::vec3 parent_position, glm::vec2 parent_size) {
	if (!display) {
		return;
	}
	glm::vec3 new_pos = position + parent_position + glm::vec3(0.0f, 0.0f, 0.01f);
	new_pos.z += 0.01f;

	std::string tex = texture_name;
	bool h = hovered();

	if (h) {
		if (!hover_texture_name.empty()) {
			tex = hover_texture_name;
		}

		if (!hover_event.path.empty() && !is_hovered) {
			audio_manager.play_one_shot_2d(hover_event);
			is_hovered = true;
		}
	}

	if (clicked()) {
		if (!click_event.path.empty()) {
			audio_manager.play_one_shot_2d(click_event);
		}
	}

	if (texture_name.empty()) {
		sprite_draw::draw_colored(new_pos, size, color, is_screen_space);
	} else {
		sprite_draw::draw_sprite(new_pos, size, color, tex.c_str(), is_screen_space);
	}

	if (text.empty()) {
		return;
	}
	// text_draw::draw_text(text, is_screen_space, new_pos + glm::vec3(0.0f, 0.0f, 0.5f), text_color, text_scale,
	// 		font_name, centered_x, centered_y, glm::vec3(0.0f));
}

bool UIButton::clicked() {
	if (!active) {
		return false;
	}
	if (!display) {
		return false;
	}

	auto mouse_pos = input_manager.get_mouse_position();
	mouse_pos.y = DisplayManager::get().get_window_size().y - mouse_pos.y;
	mouse_pos.x = mouse_pos.x;
	auto pos = position + parent_position;

	glm::vec2 x_bounds = glm::vec2(pos.x - (size.x / 2.0f), pos.x + (size.x / 2.0f));
	glm::vec2 y_bounds = glm::vec2(pos.y - (size.y / 2.0f), pos.y + (size.y / 2.0f));

	if (!texture_name.empty()) {
		auto t = SpriteManager::get()->get_sprite_texture(texture_name);
		int sprite_x_size = t.width;
		int sprite_y_size = t.height;

		float sprite_x_aspect = 1.0f;
		float sprite_y_aspect = 1.0f;

		if (sprite_x_size > sprite_y_size) {
			sprite_y_aspect = (float)sprite_y_size / (float)sprite_x_size;
		} else {
			sprite_x_aspect = (float)sprite_x_size / (float)sprite_y_size;
		}

		x_bounds = glm::vec2(pos.x - (size.x / 2.0f) * sprite_x_aspect, pos.x + (size.x / 2.0f) * sprite_x_aspect);
		y_bounds = glm::vec2(pos.y - (size.y / 2.0f) * sprite_y_aspect, pos.y + (size.y / 2.0f) * sprite_y_aspect);

		glm::vec2 s = glm::vec2(x_bounds.y - x_bounds.x, y_bounds.y - y_bounds.x);
	}

	// check if mouse is over the button
	if (mouse_pos.x > x_bounds.x && mouse_pos.x < x_bounds.y && mouse_pos.y > y_bounds.x && mouse_pos.y < y_bounds.y) {
		// check if mouse is clicked
		if (input_manager.is_action_just_pressed("mouse_left")) {
			return true;
		}
	}
	return false;
}

bool UIButton::hovered() {
	if (!active) {
		return false;
	}
	if (!display) {
		return false;
	}

	auto mouse_pos = input_manager.get_mouse_position();
	mouse_pos.y = DisplayManager::get().get_window_size().y - mouse_pos.y;
	mouse_pos.x = mouse_pos.x;
	auto pos = position + parent_position;

	glm::vec2 x_bounds = glm::vec2(pos.x - (size.x / 2.0f), pos.x + (size.x / 2.0f));
	glm::vec2 y_bounds = glm::vec2(pos.y - (size.y / 2.0f), pos.y + (size.y / 2.0f));

	if (!texture_name.empty()) {
		auto t = SpriteManager::get()->get_sprite_texture(texture_name);
		int sprite_x_size = t.width;
		int sprite_y_size = t.height;

		float sprite_x_aspect = 1.0f;
		float sprite_y_aspect = 1.0f;

		if (sprite_x_size > sprite_y_size) {
			sprite_y_aspect = (float)sprite_y_size / (float)sprite_x_size;
		} else {
			sprite_x_aspect = (float)sprite_x_size / (float)sprite_y_size;
		}

		x_bounds = glm::vec2(pos.x - (size.x / 2.0f) * sprite_x_aspect, pos.x + (size.x / 2.0f) * sprite_x_aspect);
		y_bounds = glm::vec2(pos.y - (size.y / 2.0f) * sprite_y_aspect, pos.y + (size.y / 2.0f) * sprite_y_aspect);

		glm::vec2 s = glm::vec2(x_bounds.y - x_bounds.x, y_bounds.y - y_bounds.x);
	}

	// check if mouse is over the button
	if (mouse_pos.x > x_bounds.x && mouse_pos.x < x_bounds.y && mouse_pos.y > y_bounds.x && mouse_pos.y < y_bounds.y) {
		return true;
	}
	is_hovered = false;
	return false;
}
