#include "text_draw.h"

#include "display/display_manager.h"
#include "opengl/opengl_manager.h"

const uint32_t MAX_CHARACTERS = 1000;
const uint32_t MAX_VERTEX_COUNT = 4 * MAX_CHARACTERS;
const uint32_t MAX_INDEX_COUNT = 6 * MAX_CHARACTERS;

void TextDraw::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(TextVertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDEX_COUNT * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *)nullptr);
	// vertex color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *)offsetof(TextVertex, color));
	// vertex uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *)offsetof(TextVertex, uv));
	// is screen space
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(TextVertex), (void *)offsetof(TextVertex, is_screen_space));

	glBindVertexArray(0);

	shader.load_from_files(shader_path("text.vert"), shader_path("text.frag"));
}

void TextDraw::draw() {
	if (vertices.empty()) {
		return;
	}

	// TODO: Dynamically resize buffers?
	if (vertices.size() > MAX_VERTEX_COUNT || indices.size() > MAX_INDEX_COUNT) {
		SPDLOG_ERROR("Too many characters to draw!!!");
		return;
	}

	shader.use();

	// update buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TextVertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	shader.set_int("font_atlas_map", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, FontManager::get()->fonts.begin()->second.texture);

	vertices.clear();
	indices.clear();
}

namespace text_draw {

void draw_text_2d(const std::string &text, const glm::vec2 &position, const glm::vec3 &color, float scale, Font *font) {
	draw_text(text, true, glm::vec3(position, 0.0f), color, scale, font);
}

void draw_text_3d(const std::string &text, const glm::vec3 &position, const glm::vec3 &color, float scale, Font *font) {
	draw_text(text, false, position, color, scale, font);
}

void draw_text(const std::string &text, bool is_screen_space, const glm::vec3 &position, const glm::vec3 &color,
		float scale, Font *font) {
	if (font == nullptr) {
		font = &FontManager::get()->fonts.begin()->second;
	}

	OpenglManager *opengl_manager = OpenglManager::get();
	TextDraw &text_draw = opengl_manager->text_draw;

	// We're scaling the text by arbitrary amount
	// Correct way to do it would be to calculate it based on the font size which we loaded using FreeType
	// But whatever
	scale *= 0.02f;

	float x = position.x;

	float aspect = 1.0;

	if (is_screen_space) {
		glm::vec2 window_size = DisplayManager::get()->get_framebuffer_size();
		aspect = window_size.y / window_size.x;
		// We're scaling down the font even more if it's in screenspace coords, cause they are just [-1; 1]
		scale *= 0.1f;
	}

	for (char c : text) {
		Character character = font->characters[c];

		float x_size = character.x_max - character.x_min;
		float y_size = character.y_max - character.y_min;

		float xpos = x + character.bearing.x * scale * aspect;
		float ypos = position.y - (y_size - character.bearing.y) * scale;
		float zpos = position.z;

		float w = x_size * scale * aspect;
		float h = y_size * scale;

		float uv_x_min = character.x_min / font->texture_size.x;
		float uv_x_max = character.x_max / font->texture_size.x;
		float uv_y_min = character.y_max / font->texture_size.y;
		float uv_y_max = character.y_min / font->texture_size.y;

		int ss = is_screen_space ? 1 : 0;

		//update the vertex buffer
		text_draw.vertices.push_back({ { xpos, ypos + h, zpos }, color, { uv_x_min, uv_y_max }, ss });
		text_draw.vertices.push_back({ { xpos, ypos, zpos }, color, { uv_x_min, uv_y_min }, ss });
		text_draw.vertices.push_back({ { xpos + w, ypos, zpos }, color, { uv_x_max, uv_y_min }, ss });
		text_draw.vertices.push_back({ { xpos, ypos + h, zpos }, color, { uv_x_min, uv_y_max }, ss });
		text_draw.vertices.push_back({ { xpos + w, ypos, zpos }, color, { uv_x_max, uv_y_min }, ss });
		text_draw.vertices.push_back({ { xpos + w, ypos + h, zpos }, color, { uv_x_max, uv_y_max }, ss });

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (character.advance >> 6) * scale * aspect; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
}

} //namespace text_draw