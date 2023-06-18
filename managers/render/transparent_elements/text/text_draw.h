#ifndef SILENCE_TEXT_DRAW_H
#define SILENCE_TEXT_DRAW_H

#include "font/font_manager.h"
#include "render/common/shader.h"
struct RenderScene;

struct TextVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
	int is_screen_space;
};

class TextDraw {
public:
	TextDraw() = default;
	RenderScene *r_scene;

	void draw_text_2d(const std::string &text, const glm::vec2 &position,
			const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f), float scale = 1.0f, std::string font_name = nullptr,
			bool center_x = false, bool center_y = false);

	void draw_text_3d(const std::string &text, const glm::vec3 &position,
			const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f), float scale = 1.0f, std::string font_name = nullptr,
			bool center_x = false, bool center_y = false);

	// Draw a text at "position" with "color" and "scale". If "font" is nullptr, the first loaded font will be used.
	void draw_text(const std::string &text, bool is_screen_space, const glm::vec3 &position,
			const glm::vec4 &color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), float scale = 1.0f, std::string font_name = nullptr,
			bool center_x = false, bool center_y = false, const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f),
			bool billboard = false);
};

#endif //SILENCE_TEXT_DRAW_H
