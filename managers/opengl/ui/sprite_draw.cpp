#include "sprite_draw.h"
#include "display/display_manager.h"
#include "sprite_manager.h"
#include <opengl/opengl_manager.h>

const uint32_t MAX_SPRITES = 1000;
const uint32_t MAX_VERTEX_COUNT = 4 * MAX_SPRITES;
const uint32_t MAX_INDEX_COUNT = 6 * MAX_SPRITES;

void SpriteDraw::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDEX_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)nullptr);
	// vertex color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, color));
	// vertex uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, uv));
	//textured
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, textured));
	// is screen space
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_INT, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, is_screen_space));

	glBindVertexArray(0);

	shader.load_from_files(shader_path("sprite.vert"), shader_path("sprite.frag"));
}

void SpriteDraw::draw() {
	std::vector<SpriteVertex> vertices;
	std::vector<uint32_t> indices;
	SPDLOG_INFO("rendering {} sprites", sprites.size());
	for (auto &sprite : sprites) {
		// collect all vertices and indices from sprites
		uint32_t index_offset = vertices.size();
		for (auto &vertex : sprite.vertices) {
			vertices.push_back(vertex);
		}

		for (auto &index : sprite.indices) {
			indices.push_back(index_offset + index);
		}
	}

	if (vertices.empty() || indices.empty()) {
		return;
	}

	// TODO: Dynamically resize buffers?
	if (vertices.size() > MAX_VERTEX_COUNT || indices.size() > MAX_INDEX_COUNT) {
		SPDLOG_ERROR("Too many sprites to draw!");
		return;
	}

	shader.use();

	// update buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(TextVertex), &vertices[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), &indices[0]);

	OpenglManager *opengl_manager = OpenglManager::get();
	shader.set_mat4("projection", opengl_manager->projection);
	shader.set_mat4("view", opengl_manager->view);
	shader.set_int("textured", 0);
	shader.set_int("sprite_texture", 0);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, FontManager::get()->fonts.begin()->second.texture);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	sprites.clear();
}

void sprite_draw::draw_colored(const glm::vec2 &position, const glm::vec2 &size, const glm::vec3 &color, bool is_screen_space) {

	float aspect = 1.0f;

	if (is_screen_space) {
		glm::vec2 window_size = DisplayManager::get()->get_framebuffer_size();
		aspect = window_size.y / window_size.x;
	}

	// create array of SpriteVertex
	auto manager = OpenglManager::get();
	Sprite sprite = {};
	sprite.vertices.resize(4);
	sprite.indices.resize(6);
	sprite.transform = glm::mat4(1.0f);
	sprite.texture_name = "";

	float x = position.x;
	float y = position.y;

	float w = size.x / 2.0f;
	float h = size.y / 2.0f;

	//update the vertices
	sprite.vertices[0] = { { x - w, y + h, 0.0f }, color, { 0.0f, 1.0f }, is_screen_space }; // 0
	sprite.vertices[1] = { { x - w, y - h, 0.0f }, color, { 0.0f, 0.0f }, is_screen_space }; // 1
	sprite.vertices[2] = { { x + w, y - h, 0.0f }, color, { 1.0f, 0.0f }, is_screen_space }; // 2
	sprite.vertices[3] = { { x + w, y + h, 0.0f }, color, { 1.0f, 1.0f }, is_screen_space }; // 3

	//update the indices
	uint32_t index = 0;
	sprite.indices[index++] = 0;
	sprite.indices[index++] = 1;
	sprite.indices[index++] = 2;
	sprite.indices[index++] = 0;
	sprite.indices[index++] = 2;
	sprite.indices[index++] = 3;

	manager->sprite_draw.sprites.push_back(sprite);
}

