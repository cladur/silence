#include "sprite_draw.h"
#include "display/display_manager.h"
#include "sprite_manager.h"
#include <opengl/opengl_manager.h>

const uint32_t VERTEX_COUNT = 4;
const uint32_t INDEX_COUNT = 6;

void SpriteDraw::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)nullptr);
	// vertex color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, color));
	// vertex uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, uv));
	// is screen space
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(SpriteVertex), (void *)offsetof(SpriteVertex, is_screen_space));

	glBindVertexArray(0);

	shader.load_from_files(shader_path("sprite.vert"), shader_path("sprite.frag"));
}

void SpriteDraw::draw() {
	// this is probably super inefficient, but i had no clue how to manage multiple textures in a single batched draw call

	for (auto &sprite : sprites) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sprite.vertices.size() * sizeof(SpriteVertex), &sprite.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sprite.indices.size() * sizeof(uint32_t), &sprite.indices[0]);

		auto t = SpriteManager::get()->get_sprite_texture(sprite.texture_name);
		int textured = !sprite.texture_name.empty();

		OpenglManager *opengl_manager = OpenglManager::get();
		glm::mat4 proj = opengl_manager->projection;
		if (sprite.vertices[0].is_screen_space == 1) {
			glm::vec2 window_size = DisplayManager::get()->get_framebuffer_size();
			proj = glm::ortho(0.0f, window_size.x, 0.0f, window_size.y);
		}

		shader.use();
		shader.set_mat4("projection", proj);
		shader.set_mat4("view", opengl_manager->view);
		shader.set_int("textured", textured);
		shader.set_int("sprite_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t.id);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, sprite.indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
	sprites.clear();
}

Sprite sprite_draw::default_vertex_data(
		const glm::vec2 &position,
		const glm::vec2 &size,
		float sprite_x_size,
		float sprite_y_size,
		const glm::vec3 &color,
		bool is_screen_space,
		Alignment alignment) {
	Sprite sprite = {};
	sprite.vertices.resize(4);
	sprite.indices.resize(6);
	sprite.transform = glm::mat4(1.0f);
	sprite.texture_name = "";

	float aspect = 1.0f;

	glm::vec2 aligned_position = position;

	if (is_screen_space)
	{
		glm::vec2 window_size = DisplayManager::get()->get_framebuffer_size();
		switch (alignment) {
			case Alignment::CENTER:
				aligned_position.x += window_size.x / 2.0f;
				aligned_position.y += window_size.y / 2.0f;
				break;
			case Alignment::TOP:
				aligned_position.x += window_size.x / 2.0f;
				aligned_position.y += window_size.y;
				break;
			case Alignment::BOTTOM:
				aligned_position.x += window_size.x / 2.0f;
				break;
			case Alignment::LEFT:
				aligned_position.y += window_size.y / 2.0f;
				break;
			case Alignment::RIGHT:
				aligned_position.x += window_size.x;
				aligned_position.y += window_size.y / 2.0f;
				break;
			case Alignment::TOP_LEFT:
				aligned_position.y += window_size.y;
				break;
			case Alignment::TOP_RIGHT:
				aligned_position.x += window_size.x;
				aligned_position.y += window_size.y;
				break;
			case Alignment::BOTTOM_LEFT:
				break;
			case Alignment::BOTTOM_RIGHT:
				aligned_position.x += window_size.x;
				break;
		}
	}

	float sprite_x_aspect = 1.0f;
	float sprite_y_aspect = 1.0f;

	if (sprite_x_size > sprite_y_size) {
		sprite_y_aspect = sprite_x_size / sprite_y_size;
	}
	else {
		sprite_x_aspect = sprite_y_size / sprite_x_size;
	}

	float x = aligned_position.x * aspect;
	float y = aligned_position.y;

	float w = size.x / 2.0f * aspect * sprite_y_aspect;
	float h = size.y / 2.0f * sprite_x_aspect;

	//update the vertices
	sprite.vertices[0] = { { x - w, y + h, 0.0f }, color, { 0.0f, 0.0f }, is_screen_space }; // 0
	sprite.vertices[1] = { { x - w, y - h, 0.0f }, color, { 0.0f, 1.0f }, is_screen_space }; // 1
	sprite.vertices[2] = { { x + w, y - h, 0.0f }, color, { 1.0f, 1.0f }, is_screen_space }; // 2
	sprite.vertices[3] = { { x + w, y + h, 0.0f }, color, { 1.0f, 0.0f }, is_screen_space }; // 3

	//update the indices
	uint32_t index = 0;
	sprite.indices[index++] = 0;
	sprite.indices[index++] = 1;
	sprite.indices[index++] = 2;
	sprite.indices[index++] = 0;
	sprite.indices[index++] = 2;
	sprite.indices[index++] = 3;

	return sprite;
}

void sprite_draw::draw_colored(const glm::vec2 &position, const glm::vec2 &size, const glm::vec3 &color,
		bool is_screen_space, Alignment alignment) {

	Sprite sprite = default_vertex_data(position, size, 1.0f, 1.0f, color, is_screen_space, alignment);
	auto manager = OpenglManager::get();
	manager->sprite_draw.sprites.push_back(sprite);
}

void sprite_draw::draw_sprite(const glm::vec2 &position, const glm::vec2 &size, const glm::vec3 &color,
		const char *texture_name, bool is_screen_space, Alignment alignment) {

	Texture t = SpriteManager::get()->get_sprite_texture(texture_name);

	Sprite sprite = default_vertex_data(position, size, (float)t.width, (float)t.height, color, is_screen_space, alignment);
	sprite.texture_name = texture_name;
	auto manager = OpenglManager::get();
	manager->sprite_draw.sprites.push_back(sprite);
}


