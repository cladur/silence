#include "utils.h"

#include "glad/glad.h"

namespace utils {

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quad_vao = 0;
unsigned int quad_vbo;
void render_quad() {
	if (quad_vao == 0) {
		// clang-format off
		float quad_vertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// clang-format on
		// setup plane VAO
		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	}
	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cube_vao = 0;
unsigned int cube_vbo = 0;

void render_cube() {
	// initialize (if necessary)
	if (cube_vao == 0) {
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
			// bottom face
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
			-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
		};
		glGenVertexArrays(1, &cube_vao);
		glGenBuffers(1, &cube_vbo);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cube_vao);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cube_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphere_vao = 0;
unsigned int index_count;
void render_sphere() {
	if (sphere_vao == 0) {
		glGenVertexArrays(1, &sphere_vao);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int x_segments = 16;
		const unsigned int y_segments = 16;
		const int full_size = (x_segments + 1) * (y_segments + 1);
		positions.reserve(full_size);
		uv.reserve(full_size);
		normals.reserve(full_size);
		indices.reserve(full_size);
		const float pi = 3.14159265359f;
		for (unsigned int x = 0; x <= x_segments; ++x) {
			for (unsigned int y = 0; y <= y_segments; ++y) {
				float x_segment = (float)x / (float)x_segments;
				float y_segment = (float)y / (float)y_segments;
				float x_pos = std::cos(x_segment * 2.0f * pi) * std::sin(y_segment * pi);
				float y_pos = std::cos(y_segment * pi);
				float z_pos = std::sin(x_segment * 2.0f * pi) * std::sin(y_segment * pi);

				positions.emplace_back(x_pos, y_pos, z_pos);
				uv.emplace_back(x_segment, y_segment);
				normals.emplace_back(x_pos, y_pos, z_pos);
			}
		}

		bool odd_row = false;
		for (unsigned int y = 0; y < y_segments; ++y) {
			if (!odd_row) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= x_segments; ++x) {
					indices.push_back(y * (x_segments + 1) + x);
					indices.push_back((y + 1) * (x_segments + 1) + x);
				}
			} else {
				for (int x = x_segments; x >= 0; --x) {
					indices.push_back((y + 1) * (x_segments + 1) + x);
					indices.push_back(y * (x_segments + 1) + x);
				}
			}
			odd_row = !odd_row;
		}
		index_count = static_cast<unsigned int>(indices.size());

		std::vector<float> data;
		data.reserve(full_size * 3);
		for (unsigned int i = 0; i < positions.size(); ++i) {
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (!normals.empty()) {
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (!uv.empty()) {
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}
		glBindVertexArray(sphere_vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		unsigned int stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
	}

	glBindVertexArray(sphere_vao);
	glDrawElements(GL_TRIANGLE_STRIP, index_count, GL_UNSIGNED_INT, 0);
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int cone_vao = 0;
int indices_count = 0;
void render_cone() {
	if (cone_vao == 0) {
		glGenVertexArrays(1, &cone_vao);

		unsigned int vbo;
		glGenBuffers(1, &vbo);

		float length = 1.0f;
		float cone_radius = 1.0f;
		int sides = 32;

		const float PI = 3.14159265359f;
		const float sector_step = 2.0f * PI / (float)sides;

		std::vector<float> data;

		for (int i = 1; i < sides + 1; i++) {
			// triangle fan, bottom
			data.insert(data.end(),
					{ 0.0f, 0.0f, -length, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }); // center point; position, normal, uv

			data.insert(data.end(),
					{ (cone_radius * cos((float)(i + 1) * sector_step)),
							(cone_radius * sin((float)(i + 1) * sector_step)), -length, 0.0f, 1.0f, 0.0f, 0.0f,
							0.0f }); // second outer point
			data.insert(data.end(),
					{ cone_radius * cos((float)i * sector_step), cone_radius * sin((float)i * sector_step), -length,
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f }); // first outer point

			// side triangle + point
			data.insert(data.end(),
					{ (cone_radius * cos((float)i * sector_step)), (cone_radius * sin((float)i * sector_step)), -length,
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f });
			data.insert(data.end(),
					{ (cone_radius * cos((float)(i + 1) * sector_step)),
							(cone_radius * sin((float)(i + 1) * sector_step)), -length, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f });

			data.insert(data.end(), { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f }); // origin, peak
			indices_count += 6;
		}

		glBindVertexArray(cone_vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		int stride = (3 + 3 + 2) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
	}

	glBindVertexArray(cone_vao);
	glDrawArrays(GL_TRIANGLES, 0, indices_count);
}

} //namespace utils