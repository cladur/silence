#ifndef SILENCE_UI_BUTTON_H
#define SILENCE_UI_BUTTON_H

#include "ui_element.h"

#include "audio/event_reference.h"

class UIButton : public UIElement {
private:
	bool is_hovered = false;
public:
	std::string text;
	std::string font_name;
	glm::vec3 text_color = glm::vec3(1.0f);
	float text_scale = 1.0f;
	bool centered_x = true;
	bool centered_y = true;
	bool active = true;
	std::string hover_texture_name;

	EventReference hover_event;
	EventReference click_event;

	UIButton() = default;
	UIButton(
			glm::vec3 position,
			glm::vec2 size,
			const std::string& text,
			const std::string& font_name,
			const std::string &texture_name,
			const std::string &hover_sound_name = "",
			const std::string &click_sound_name = "");

	void draw() override;
	void draw(glm::vec3 parent_position, glm::vec2 parent_size) override;

	bool clicked();
	bool hovered();
};

#endif //SILENCE_UI_BUTTON_H
