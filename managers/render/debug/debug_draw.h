
#ifndef SILENCE_DEBUG_DRAW_H
#define SILENCE_DEBUG_DRAW_H

#include "managers/render/common/shader.h"

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
	glm::mat4 view, projection;

	void startup();
	void draw();

	// Draw a line from "from" to "to". Beginning of the line is red turns to blue at the end.
	void draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

	// Draw a box with center at "center" and scale "scale".
	void draw_box(const glm::vec3 &center, const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f),
			const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f), const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

	void draw_box(const glm::vec3 &center, const glm::quat &orientation,
			const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f), const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

	// Draw a sphere with center at "center" and radius "radius".
	void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));

	void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color, int segments);

	void draw_frustum(const glm::vec3 &center, const glm::quat &orientation, float fov, float aspect, float near,
			float far, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));
	void draw_arrow(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));
	void draw_arrow(const glm::vec3 &from, const glm::vec3 &to, float length,
			const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f));
	void draw_cone(const glm::vec3 &from, const glm::vec3 &to, float radius,
			const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f), int num_of_segments = 32);
	void draw_cone(const glm::vec3 &from, const glm::vec3 &to, float length, float radius,
			const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f), int num_of_segments = 32);
	void draw_circle(const glm::vec3 &center, glm::vec3 direction, float radius,
			const glm::vec3 &color = glm::vec3(0.0f, 1.0f, 0.0f), int num_of_segments = 32);

	//void draw_cone(const glm::vec3 &origin, const glm::vec3 &direction, float length, float radius, const glm::vec3 &color);
};

#endif //SILENCE_DEBUG_DRAW_H
