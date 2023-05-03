#include "debug_draw.h"

#include "opengl/opengl_manager.h"

const uint32_t MAX_VERTEX_COUNT = 10000;

void DebugDraw::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(DebugVertex), nullptr, GL_DYNAMIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void *)nullptr);
	// vertex color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void *)offsetof(DebugVertex, color));

	glBindVertexArray(0);

	shader.load_from_files(shader_path("debug.vert"), shader_path("debug.frag"));
}

void DebugDraw::draw() {
	if (vertices.empty()) {
		return;
	}

	// TODO: Dynamically resize buffers?
	if (vertices.size() > MAX_VERTEX_COUNT) {
		SPDLOG_ERROR("Too many debug vertices to draw!!!");
		return;
	}

	shader.use();

	// update buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(DebugVertex), &vertices[0]);

	OpenglManager *opengl_manager = OpenglManager::get();
	shader.set_mat4("projection", opengl_manager->projection);
	shader.set_mat4("view", opengl_manager->view);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);

	vertices.clear();
}

namespace debug_draw {

void draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color) {
	OpenglManager *opengl_manager = OpenglManager::get();

	opengl_manager->debug_draw.vertices.push_back({ from, color });
	opengl_manager->debug_draw.vertices.push_back({ to, color });
}

// Draw a box with center at "center" and scale "scale".
void draw_box(const glm::vec3 &center, const glm::vec3 &scale, const glm::vec3 &color) {
	// Generate a box mesh, using draw_line to draw the lines
	glm::vec3 vertices[8] = { glm::vec3(
									  center.x - scale.x / 2.0f, center.y - scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y - scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y + scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x - scale.x / 2.0f, center.y + scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x - scale.x / 2.0f, center.y - scale.y / 2.0f, center.z + scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y - scale.y / 2.0f, center.z + scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y + scale.y / 2.0f, center.z + scale.z / 2.0f),
		glm::vec3(center.x - scale.x / 2.0f, center.y + scale.y / 2.0f, center.z + scale.z / 2.0f) };

	draw_line(vertices[0], vertices[1], color);
	draw_line(vertices[1], vertices[2], color);
	draw_line(vertices[2], vertices[3], color);
	draw_line(vertices[3], vertices[0], color);
	draw_line(vertices[4], vertices[5], color);
	draw_line(vertices[5], vertices[6], color);
	draw_line(vertices[6], vertices[7], color);
	draw_line(vertices[7], vertices[4], color);
	draw_line(vertices[0], vertices[4], color);
	draw_line(vertices[1], vertices[5], color);
	draw_line(vertices[2], vertices[6], color);
	draw_line(vertices[3], vertices[7], color);
}

// Draw a sphere with center at "center" and radius "radius".
void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color) {
	// Generate a sphere mesh, using draw_line to draw the lines
	const int num_segments = 8; // number of horizontal segments
	const int num_rings = 4; // number of vertical rings
	const float pi = 3.14159265358979323846;
	const float delta_ring = pi / num_rings;
	const float delta_segment = 2.0f * pi / num_segments;

	// generate vertices
	float vertices[num_rings + 1][num_segments + 1][3];
	for (int i = 0; i <= num_rings; i++) {
		float r = radius * sin(i * delta_ring);
		for (int j = 0; j <= num_segments; j++) {
			vertices[i][j][0] = r * cos(j * delta_segment) + center.x;
			vertices[i][j][1] = radius * cos(i * delta_ring) + center.y;
			vertices[i][j][2] = r * sin(j * delta_segment) + center.z;
		}
	}

	// generate line segments
	for (int i = 0; i < num_rings; i++) {
		for (int j = 0; j < num_segments; j++) {
			int i1 = i;
			int i2 = i + 1;
			int j1 = j;
			int j2 = (j + 1) % num_segments;
			draw_line(glm::vec3(vertices[i1][j1][0], vertices[i1][j1][1], vertices[i1][j1][2]),
					glm::vec3(vertices[i2][j1][0], vertices[i2][j1][1], vertices[i2][j1][2]), color);
			draw_line(glm::vec3(vertices[i2][j1][0], vertices[i2][j1][1], vertices[i2][j1][2]),
					glm::vec3(vertices[i2][j2][0], vertices[i2][j2][1], vertices[i2][j2][2]), color);
		}
	}
}

}; //namespace debug_draw