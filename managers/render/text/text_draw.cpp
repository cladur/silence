#include "text_draw.h"

#include "display/display_manager.h"

#include "render/vk_debug.h"
#include <render/render_manager.h>
#include <render/vk_initializers.h>
#include <vulkan/vulkan_handles.hpp>

VertexInputDescription TextVertex::get_vertex_description() {
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	vk::VertexInputBindingDescription main_binding = {};
	main_binding.binding = 0;
	main_binding.stride = sizeof(TextVertex);
	main_binding.inputRate = vk::VertexInputRate::eVertex;

	description.bindings.push_back(main_binding);

	//Position will be stored at Location 0
	vk::VertexInputAttributeDescription position_attribute = {};
	position_attribute.binding = 0;
	position_attribute.location = 0;
	position_attribute.format = vk::Format::eR32G32B32Sfloat;
	position_attribute.offset = offsetof(TextVertex, position);

	//UV will be stored at Location 1
	vk::VertexInputAttributeDescription uv_attribute = {};
	uv_attribute.binding = 0;
	uv_attribute.location = 1;
	uv_attribute.format = vk::Format::eR32G32Sfloat;
	uv_attribute.offset = offsetof(TextVertex, uv);

	//Color will be stored at Location 2
	vk::VertexInputAttributeDescription color_attribute = {};
	color_attribute.binding = 0;
	color_attribute.location = 2;
	color_attribute.format = vk::Format::eR32G32B32Sfloat;
	color_attribute.offset = offsetof(TextVertex, color);

	//Is screen space will be stored at Location 3
	vk::VertexInputAttributeDescription is_screen_space_attribute = {};
	is_screen_space_attribute.binding = 0;
	is_screen_space_attribute.location = 3;
	is_screen_space_attribute.format = vk::Format::eR32Sint;
	is_screen_space_attribute.offset = offsetof(TextVertex, is_screen_space);

	description.attributes.push_back(position_attribute);
	description.attributes.push_back(uv_attribute);
	description.attributes.push_back(color_attribute);
	description.attributes.push_back(is_screen_space_attribute);

	return description;
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
	auto render_manager = RenderManager::get();

	if (font == nullptr) {
		font = &FontManager::get()->fonts.begin()->second;
	}

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
		render_manager->text_vertices.push_back({ { xpos, ypos + h, zpos }, { uv_x_min, uv_y_max }, color, ss });
		render_manager->text_vertices.push_back({ { xpos, ypos, zpos }, { uv_x_min, uv_y_min }, color, ss });
		render_manager->text_vertices.push_back({ { xpos + w, ypos, zpos }, { uv_x_max, uv_y_min }, color, ss });
		render_manager->text_vertices.push_back({ { xpos, ypos + h, zpos }, { uv_x_min, uv_y_max }, color, ss });
		render_manager->text_vertices.push_back({ { xpos + w, ypos, zpos }, { uv_x_max, uv_y_min }, color, ss });
		render_manager->text_vertices.push_back({ { xpos + w, ypos + h, zpos }, { uv_x_max, uv_y_max }, color, ss });

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (character.advance >> 6) * scale * aspect; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
}

}; //namespace text_draw