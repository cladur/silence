#ifndef SILENCE_SPRITE_DRAW_H
#define SILENCE_SPRITE_DRAW_H

#include <opengl/shader.h>
struct SpriteVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
	int is_screen_space;
};

struct Sprite {
	std::vector<SpriteVertex> vertices;
	glm::vec2 size;
	glm::vec3 position;
	std::vector<uint32_t> indices;
	bool billboard = false;
	glm::mat4 transform;
	std::string texture_name;
};

class SpriteDraw {
private:
	//  render data
	unsigned int vao, vbo, ebo;
	Shader shader;

public:
	std::vector<Sprite> sprites;

	void startup();
	void draw();
};

namespace sprite_draw {

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

Sprite default_vertex_data(const glm::vec3 &position, const glm::vec2 &size, float sprite_x_size, float sprite_y_size,
		const glm::vec3 &color, bool is_screen_space, Alignment alignment = Alignment::CENTER);

// draws a single colored quad. If is screen spaced, the scale represents pixel scale.
void draw_colored(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, bool is_screen_space,
		Alignment alignment = Alignment::CENTER);
void draw_sprite(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, const char *texture_name,
		bool is_screen_space, Alignment alignment = Alignment::CENTER);
void draw_sprite_billboard(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, const char *texture_name);

}
#endif //SILENCE_SPRITE_DRAW_H
