#ifndef SILENCE_SPRITE_DRAW_H
#define SILENCE_SPRITE_DRAW_H

#include <opengl/shader.h>
struct SpriteVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
	int textured;
	int is_screen_space;
};

struct Sprite {
	std::vector<SpriteVertex> vertices;
	std::vector<uint32_t> indices;
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
	// draws a single colored quad. If is screen spaced, the scale represents pixel scale.
	void draw_colored(const glm::vec2 &position, const glm::vec2 &size, const glm::vec3 &color, bool is_screen_space);
}

#endif //SILENCE_SPRITE_DRAW_H
