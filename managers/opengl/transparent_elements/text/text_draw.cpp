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

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_COUNT * sizeof(TextVertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDEX_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

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
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(TextVertex), (void *)offsetof(TextVertex, is_screen_space));

	glBindVertexArray(0);

	shader.load_from_files(shader_path("text.vert"), shader_path("text.frag"));
}

void TextDraw::draw() {
	ZoneScoped;
	if (vertices.empty() || indices.empty()) {
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
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(TextVertex), &vertices[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(uint32_t), &indices[0]);

	OpenglManager *opengl_manager = OpenglManager::get();
	shader.set_mat4("projection", opengl_manager->projection);
	shader.set_mat4("view", opengl_manager->view);
	shader.set_int("font_atlas_map", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, FontManager::get()->fonts.begin()->second.texture.id);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	vertices.clear();
	indices.clear();
}

namespace text_draw {

void draw_text_2d(const std::string &text, const glm::vec2 &position, const glm::vec3 &color, float scale,  std::string font_name,
		bool center_x, bool center_y) {
	draw_text(text, true, glm::vec3(position, 0.0f), color, scale, font_name);
}

void draw_text_3d(const std::string &text, const glm::vec3 &position, const glm::vec3 &color, float scale,  std::string font_name,
		bool center_x, bool center_y) {
	draw_text(text, false, position, color, scale, font_name, center_x, center_y);
}

void draw_text(const std::string &text, bool is_screen_space, const glm::vec3 &position, const glm::vec3 &color,
		float scale, std::string font_name, bool center_x, bool center_y, const glm::vec3 &rotation) {

	auto font_manager = FontManager::get();

	TransparentObject object = {};
	object.transform = glm::mat4(1.0f);
	object.position = position;
	object.texture_name = "";

	Font *font = &font_manager->fonts.begin()->second;
	if (font_name.empty()) {
		font = &font_manager->fonts.begin()->second;
		object.texture_name = font_manager->fonts.begin()->first;
	} else {
		font = &font_manager->fonts[font_name];
		object.texture_name = font_name;
	}

	OpenglManager *opengl_manager = OpenglManager::get();
	//TextDraw &text_draw = opengl_manager->text_draw;

	// We're scaling the text by arbitrary amount
	// Correct way to do it would be to calculate it based on the font size which we loaded using FreeType
	// But whatever
	//scale *= 0.02f;

	// We're scaling down the font even more if it's in screenspace coords, cause they are just [-1; 1]
	float screen_space_scale = 1.0f;

	float aspect = 1.0;

	if (!is_screen_space) {
		glm::vec2 window_size = DisplayManager::get()->get_framebuffer_size();
		aspect = 1.0f;//window_size.y / window_size.x;
		// We're scaling down the font even more if it's in screenspace coords, cause they are just [-1; 1]
		scale *= 0.02f;
	}

	float x = position.x;
	if (center_x) {
		float text_width = 0.0f;
		for (char c : text) {
			Character character = font->characters[c];
			if (is_screen_space) {
				//text_width += (character.advance >> 6) * scale * screen_space_scale / 2;
				text_width += (character.advance >> 6) * scale;
			} else {
				text_width += (character.advance >> 6) * scale;
			}
		}
		x -= text_width / 2.0f;
	}

	float y = position.y;
	if (center_y) {
		float text_height = 0.0f;
		for (char c : text) {
			Character character = font->characters[c];
			if (text_height < (character.y_max - character.y_min) * scale) {
				if (is_screen_space) {
					text_height = (character.y_max - character.y_min) * scale;
				} else {
					text_height = (character.y_max - character.y_min) * scale;
				}
			}
		}
		y -= text_height / 2.0f;
	}

	//creating rotation matrix
	glm::mat4 rotation_matrix = glm::mat4(1.0f);
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	for (char c : text) {
		Character character = font->characters[c];

		float x_size = character.x_max - character.x_min;
		float y_size = character.y_max - character.y_min;

		float xpos = x + character.bearing.x * scale * aspect;
		float ypos = y - (y_size - character.bearing.y) * scale;
		float zpos = position.z;

		float w = x_size * scale * aspect;
		float h = y_size * scale;

		float uv_x_min = character.x_min / font->texture_size.x;
		float uv_x_max = character.x_max / font->texture_size.x;
		float uv_y_min = character.y_max / font->texture_size.y;
		float uv_y_max = character.y_min / font->texture_size.y;

		int ss = is_screen_space ? 1 : 0;

		glm::vec4 v1 = { xpos, ypos + h, zpos, 0.0f };
		glm::vec4 v2 = { xpos, ypos, zpos, 0.0f };
		glm::vec4 v3 = { xpos + w, ypos, zpos, 0.0f };
		glm::vec4 v4 = { xpos + w, ypos + h, zpos, 0.0f };

		//rotate vertices
		v1 = rotation_matrix * v1;
		v2 = rotation_matrix * v2;
		v3 = rotation_matrix * v3;
		v4 = rotation_matrix * v4;

		//update the vertices
		object.vertices.push_back({ { v1.x, v1.y, v1.z }, color, { uv_x_min, uv_y_max }, ss }); // 0
		object.vertices.push_back({ { v2.x, v2.y, v2.z }, color, { uv_x_min, uv_y_min }, ss }); // 1
		object.vertices.push_back({ { v3.x, v3.y, v3.z }, color, { uv_x_max, uv_y_min }, ss }); // 2
		object.vertices.push_back({ { v4.x, v4.y, v4.z }, color, { uv_x_max, uv_y_max }, ss }); // 3

		//update the indices
		int index = object.vertices.size() - 4;
		object.indices.push_back(0 + index);
		object.indices.push_back(1 + index);
		object.indices.push_back(2 + index);
		object.indices.push_back(0 + index);
		object.indices.push_back(2 + index);
		object.indices.push_back(3 + index);

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (character.advance >> 6) * scale * aspect; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	opengl_manager->transparent_draw.objects.push_back(object);
}

} //namespace text_draw