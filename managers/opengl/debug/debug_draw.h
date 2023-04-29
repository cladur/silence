#ifndef SILENCE_DEBUG_DRAW_H
#define SILENCE_DEBUG_DRAW_H

#include "opengl/shader.h"

class RenderManager;

struct DebugVertex {
	glm::vec3 position;
	glm::vec3 color;
};

class DebugDraw {
private:
	//  render data
	unsigned int vao, vbo, ebo;
	Shader shader;

public:
	std::vector<DebugVertex> vertices;

	void startup();
	void draw();
};

namespace debug_draw {

// Draw a line from "from" to "to". Beginning of the line is red turns to blue at the end.
void draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

// Draw a box with center at "center" and scale "scale".
void draw_box(const glm::vec3 &center, const glm::vec3 &scale, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

// Draw a sphere with center at "center" and radius "radius".
void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

}; //namespace debug_draw

#endif //SILENCE_DEBUG_DRAW_H
