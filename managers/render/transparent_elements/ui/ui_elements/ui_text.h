#ifndef SILENCE_UI_TEXT_H
#define SILENCE_UI_TEXT_H

#include "ui_element.h"

class UIText : public UIElement {
public:
	std::string text;
	bool centered_x = false;
	bool centered_y = false;

	UIText() = default;
	UIText(glm::vec3 position, float scale, const std::string& text, const std::string& font_name);

	void draw() override;
	void draw(glm::vec3 parent_position, glm::vec2 parent_size) override;
};

#endif //SILENCE_UI_TEXT_H
