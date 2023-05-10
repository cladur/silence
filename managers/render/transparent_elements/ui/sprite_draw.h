#ifndef SILENCE_SPRITE_DRAW_H
#define SILENCE_SPRITE_DRAW_H

#include "render/transparent_elements/transparent_object.h"
#include <render/common/shader.h>
struct RenderScene;

enum class Alignment {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	CENTER,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	NONE,
};

enum class SliderAlignment { LEFT_TO_RIGHT, RIGHT_TO_LEFT, TOP_TO_BOTTOM, BOTTOM_TO_TOP };

class SpriteDraw {
public:
	SpriteDraw() = default;
	RenderScene *r_scene;

	TransparentObject default_vertex_data(const glm::vec3 &position, const glm::vec2 &size, float sprite_x_size,
			float sprite_y_size, const glm::vec3 &color, bool is_screen_space, Alignment alignment = Alignment::CENTER);

	// draws a single colored quad. If is screen spaced, the scale represents pixel scale.
	void draw_colored(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, bool is_screen_space,
			Alignment alignment = Alignment::NONE);
	void draw_colored_billboard(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color);
	void draw_sprite(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, const char *texture_name,
			bool is_screen_space, Alignment alignment = Alignment::NONE);
	void draw_sprite_billboard(
			const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, const char *texture_name);

	void draw_slider_billboard(const glm::vec3 &position, float add_z, const glm::vec2 &size, const glm::vec3 &color,
			float value, SliderAlignment slider_alignment);

	void draw_sprite_scene(RenderScene *scene, const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, const char *texture_name,
			bool is_screen_space, Alignment alignment = Alignment::NONE);
};

#endif //SILENCE_SPRITE_DRAW_H
