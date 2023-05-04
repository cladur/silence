#ifndef SILENCE_TRANSPARENT_DRAW_H
#define SILENCE_TRANSPARENT_DRAW_H

#include "render/common/shader.h"
struct RenderScene;

struct TransparentVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
	int is_screen_space;
};

enum class TransparentType {
	TEXT,
	SPRITE,
};

struct TransparentObject {
	std::vector<TransparentVertex> vertices;
	std::vector<uint32_t> indices;
	glm::vec2 size;
	glm::vec3 position;
	glm::mat4 transform;
	bool billboard = false;
	TransparentType type;
	std::string texture_name; // used to find either atlas for fonts or sprites
};

class TransparentDraw {
private:
	unsigned int vao, vbo, ebo;
	Shader shader;

public:
	std::vector<TransparentObject> objects;

	void startup();
	void draw(RenderScene &render_scene);
};

#endif //SILENCE_TRANSPARENT_DRAW_H
