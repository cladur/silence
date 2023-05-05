#include "text_draw.h"

#include "display/display_manager.h"
#include "render/render_manager.h"

const uint32_t MAX_CHARACTERS = 1000;
const uint32_t MAX_VERTEX_COUNT = 4 * MAX_CHARACTERS;
const uint32_t MAX_INDEX_COUNT = 6 * MAX_CHARACTERS;

void TextDraw::draw_text_2d(const std::string &text, const glm::vec2 &position, const glm::vec3 &color, float scale,
		std::string font_name, bool center_x, bool center_y) {
	draw_text(text, true, glm::vec3(position, 0.0f), color, scale, font_name);
}

void TextDraw::draw_text_3d(const std::string &text, const glm::vec3 &position, const glm::vec3 &color, float scale,
		std::string font_name, bool center_x, bool center_y) {
	draw_text(text, false, position, color, scale, font_name, center_x, center_y);
}

void TextDraw::draw_text(const std::string &text, bool is_screen_space, const glm::vec3 &position,
		const glm::vec3 &color, float scale, std::string font_name, bool center_x, bool center_y,
		const glm::vec3 &rotation, bool billboard) {
	auto font_manager = FontManager::get();

	TransparentObject object = {};
	object.transform = glm::mat4(1.0f);
	object.texture_name = "";

	Font *font = &font_manager.fonts.begin()->second;
	if (font_name.empty()) {
		font = &font_manager.fonts.begin()->second;
		object.texture_name = font_manager.fonts.begin()->first;
	} else {
		font = &font_manager.fonts[font_name];
		object.texture_name = font_name;
	}

	// We're scaling down the font even more if it's in screenspace coords, cause they are just [-1; 1]
	float screen_space_scale = 1.0f;

	float aspect = 1.0;

	if (!is_screen_space) {
		aspect = current_scene->render_extent.x / current_scene->render_extent.y;
		// We're scaling down the font even more if it's in screenspace coords, cause they are just [-1; 1]
		scale *= 0.02f;
	}

	float x = position.x;
	float y = position.y;
	float z = position.z;
	if (billboard) {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

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

	glm::vec2 text_size = glm::vec2(0.0f);
	text_size.x = x;
	text_size.y = y;

	for (char c : text) {
		Character character = font->characters[c];

		float x_size = character.x_max - character.x_min;
		float y_size = character.y_max - character.y_min;

		float xpos = x + character.bearing.x * scale;
		float ypos = y - (y_size - character.bearing.y) * scale;
		float zpos = z;

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
		object.vertices.push_back({ { v1.x, v1.y, v1.z }, color, { uv_x_min, uv_y_max }, ss });
		object.vertices.push_back({ { v2.x, v2.y, v2.z }, color, { uv_x_min, uv_y_min }, ss });
		object.vertices.push_back({ { v3.x, v3.y, v3.z }, color, { uv_x_max, uv_y_min }, ss });
		object.vertices.push_back({ { v4.x, v4.y, v4.z }, color, { uv_x_max, uv_y_max }, ss });

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
	text_size.x = abs(x - text_size.x);

	object.type = TransparentType::TEXT;
	object.billboard = true;
	object.size = text_size / 2.0f;
	object.position = position;

	current_scene->transparent_objects.push_back(object);
}
