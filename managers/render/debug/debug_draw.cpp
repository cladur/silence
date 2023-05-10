#include "debug_draw.h"

#include "render/render_manager.h"

const uint32_t MAX_VERTEX_COUNT = 10000;

void DebugDraw::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(DebugVertex), nullptr, GL_DYNAMIC_DRAW);

	// vertex translation
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

	shader.set_mat4("projection", projection);
	shader.set_mat4("view", view);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);

	vertices.clear();
}

void DebugDraw::draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color) {
	vertices.push_back({ from, color });
	vertices.push_back({ to, color });
}

// Draw a box with center at "center" and scale "scale".
void DebugDraw::draw_box(
		const glm::vec3 &center, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec3 &color) {
	// Generate a box mesh, using draw_line to draw the lines
	glm::vec3 vertices[8] = { glm::vec3(-scale.x / 2.0f, -scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, -scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, +scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(-scale.x / 2.0f, +scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(-scale.x / 2.0f, -scale.y / 2.0f, +scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, -scale.y / 2.0f, +scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, +scale.y / 2.0f, +scale.z / 2.0f),
		glm::vec3(-scale.x / 2.0f, +scale.y / 2.0f, +scale.z / 2.0f) };

	glm::mat4 mat = glm::mat4(1.0f);

	mat = glm::translate(mat, center);

	mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	for (auto &v : vertices) {
		v = glm::vec3(mat * glm::vec4(v, 1.0f));
	}

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
void DebugDraw::draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color) {
	// Generate a sphere mesh, using draw_line to draw the lines
	const int num_segments = 12; // number of horizontal segments
	const int num_rings = 6; // number of vertical rings
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
void DebugDraw::draw_box(
		const glm::vec3 &center, const glm::quat &orientation, const glm::vec3 &scale, const glm::vec3 &color) {
	glm::vec3 vertices[8] = { glm::vec3(-scale.x / 2.0f, -scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, -scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, +scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(-scale.x / 2.0f, +scale.y / 2.0f, -scale.z / 2.0f),
		glm::vec3(-scale.x / 2.0f, -scale.y / 2.0f, +scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, -scale.y / 2.0f, +scale.z / 2.0f),
		glm::vec3(+scale.x / 2.0f, +scale.y / 2.0f, +scale.z / 2.0f),
		glm::vec3(-scale.x / 2.0f, +scale.y / 2.0f, +scale.z / 2.0f) };

	glm::mat4 mat = glm::mat4(1.0f);

	mat = glm::translate(mat, center);

	mat = mat * glm::mat4_cast(orientation);

	for (auto &v : vertices) {
		v = glm::vec3(mat * glm::vec4(v, 1.0f));
	}

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

void DebugDraw::draw_frustum(const glm::vec3 &center, const glm::quat &orientation, float fov, float aspect, float near,
		float far, const glm::vec3 &color) {
	// Calculate the four corners of the near plane of the frustum
	float tan_half_fov = tan(glm::radians(fov) / 2);
	float near_height = 2 * near * tan_half_fov;
	float near_width = near_height * aspect;
	glm::vec3 near_top_left = center + orientation * glm::vec3(-near_width / 2, near_height / 2, -near);
	glm::vec3 near_top_right = center + orientation * glm::vec3(near_width / 2, near_height / 2, -near);
	glm::vec3 near_bottom_left = center + orientation * glm::vec3(-near_width / 2, -near_height / 2, -near);
	glm::vec3 near_bottom_right = center + orientation * glm::vec3(near_width / 2, -near_height / 2, -near);

	// Calculate the four corners of the far plane of the frustum
	float far_height = 2 * far * tan_half_fov;
	float far_width = far_height * aspect;
	glm::vec3 far_top_left = center + orientation * glm::vec3(-far_width / 2, far_height / 2, -far);
	glm::vec3 far_top_right = center + orientation * glm::vec3(far_width / 2, far_height / 2, -far);
	glm::vec3 far_bottom_left = center + orientation * glm::vec3(-far_width / 2, -far_height / 2, -far);
	glm::vec3 far_bottom_right = center + orientation * glm::vec3(far_width / 2, -far_height / 2, -far);

	// Draw the lines of the frustum using the draw_line function
	// Near to Far lines
	draw_line(near_top_left, far_top_left, color);
	draw_line(near_top_right, far_top_right, color);
	draw_line(near_bottom_left, far_bottom_left, color);
	draw_line(near_bottom_right, far_bottom_right, color);
	// Near rectangle
	draw_line(near_top_left, near_top_right, color);
	draw_line(near_top_right, near_bottom_right, color);
	draw_line(near_bottom_right, near_bottom_left, color);
	draw_line(near_bottom_left, near_top_left, color);
	// Far rectangle
	draw_line(far_top_left, far_top_right, color);
	draw_line(far_top_right, far_bottom_right, color);
	draw_line(far_bottom_right, far_bottom_left, color);
	draw_line(far_bottom_left, far_top_left, color);
}
