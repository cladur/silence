#include "sprite_draw.h"
#include "display/display_manager.h"
#include "sprite_manager.h"
#include <render/render_manager.h>

const uint32_t VERTEX_COUNT = 4;
const uint32_t INDEX_COUNT = 6;

TransparentObject SpriteDraw::default_vertex_data(const glm::vec3 &position, const glm::vec2 &size,
		float sprite_x_size, float sprite_y_size, const glm::vec3 &color, bool is_screen_space, Alignment alignment) {
	TransparentObject sprite = {};
	sprite.position = position;
	sprite.vertices.resize(4);
	sprite.indices.resize(6);
	sprite.transform = glm::mat4(1.0f);
	sprite.texture_name = "";
	sprite.type = TransparentType::SPRITE;

	float aspect = 1.0f;

	glm::vec2 aligned_position = position;

	if (is_screen_space && alignment != Alignment::NONE) {
		glm::vec2 window_size = DisplayManager::get().get_framebuffer_size();
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
			default:
				break;
		}
	}

	float sprite_x_aspect = 1.0f;
	float sprite_y_aspect = 1.0f;

	if (sprite_x_size > sprite_y_size) {
		sprite_y_aspect = sprite_y_size / sprite_x_size;
	} else {
		sprite_x_aspect = sprite_x_size / sprite_y_size;
	}

	float x = aligned_position.x * aspect;
	float y = aligned_position.y;
	float z = position.z;

	float w = size.x / 2.0f * aspect * sprite_x_aspect;
	float h = size.y / 2.0f * sprite_y_aspect;

	//update the vertices
	sprite.vertices[0] = { { x - w, y + h, z }, color, { 0.0f, 0.0f }, is_screen_space }; // 0
	sprite.vertices[1] = { { x - w, y - h, z }, color, { 0.0f, 1.0f }, is_screen_space }; // 1
	sprite.vertices[2] = { { x + w, y - h, z }, color, { 1.0f, 1.0f }, is_screen_space }; // 2
	sprite.vertices[3] = { { x + w, y + h, z }, color, { 1.0f, 0.0f }, is_screen_space }; // 3

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

void SpriteDraw::draw_colored(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color,
		bool is_screen_space, Alignment alignment) {
	TransparentObject sprite = default_vertex_data(position, size, 1.0f, 1.0f, color, is_screen_space, alignment);

	current_scene->transparent_objects.push_back(sprite);
}

void SpriteDraw::draw_sprite(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color,
		const char *texture_name, bool is_screen_space, Alignment alignment) {
	Texture t = SpriteManager::get()->get_sprite_texture(texture_name);

	TransparentObject sprite =
			default_vertex_data(position, size, (float)t.width, (float)t.height, color, is_screen_space, alignment);
	sprite.texture_name = texture_name;

	current_scene->transparent_objects.push_back(sprite);
}

void SpriteDraw::draw_sprite_billboard(
		const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color, const char *texture_name) {
	Texture t = SpriteManager::get()->get_sprite_texture(texture_name);

	TransparentObject sprite = default_vertex_data(
			glm::vec3(0.0f), size, (float)t.width, (float)t.height, color, false, Alignment::NONE);

	sprite.texture_name = texture_name;
	sprite.billboard = true;
	sprite.size = size / 2.0f;
	sprite.position = position;

	current_scene->transparent_objects.push_back(sprite);
}

void SpriteDraw::draw_colored_billboard(const glm::vec3 &position, const glm::vec2 &size, const glm::vec3 &color) {
	TransparentObject sprite =
			default_vertex_data(glm::vec3(0.0f), size, 1.0f, 1.0f, color, false, Alignment::NONE);

	sprite.billboard = true;
	sprite.size = size / 2.0f;
	sprite.position = position;

	current_scene->transparent_objects.push_back(sprite);
}

void SpriteDraw::draw_slider_billboard(const glm::vec3 &position, float add_z, const glm::vec2 &size,
		const glm::vec3 &color, float value, SliderAlignment slider_alignment) {
	TransparentObject sprite = {};
	sprite.vertices.resize(4);
	sprite.indices.resize(6);
	sprite.transform = glm::mat4(1.0f);
	sprite.texture_name = "";
	sprite.position = position;

	float x = 0.0f;
	float y = 0.0f;
	float z = add_z;

	float w = size.x / 2.0f;
	float h = size.y / 2.0f;

	switch (slider_alignment) {
		case SliderAlignment::LEFT_TO_RIGHT:
			sprite.vertices[0] = { { x - w, y + h, z }, color, { 0.0f, 0.0f }, false };
			sprite.vertices[1] = { { x - w, y - h, z }, color, { 0.0f, 1.0f }, false };
			sprite.vertices[2] = { { x - w + (size.x * value), y - h, z }, color, { 1.0f, 1.0f }, false };
			sprite.vertices[3] = { { x - w + (size.x * value), y + h, z }, color, { 1.0f, 0.0f }, false };
			break;
		case SliderAlignment::RIGHT_TO_LEFT:
			sprite.vertices[0] = { { x + w, y + h, z }, color, { 0.0f, 0.0f }, false };
			sprite.vertices[1] = { { x + w, y - h, z }, color, { 0.0f, 1.0f }, false };
			sprite.vertices[2] = { { x + w - (size.x * value), y - h, z }, color, { 1.0f, 1.0f }, false };
			sprite.vertices[3] = { { x + w - (size.x * value), y + h, z }, color, { 1.0f, 0.0f }, false };
			break;
		case SliderAlignment::TOP_TO_BOTTOM:
			sprite.vertices[0] = { { x - w, y + h, z }, color, { 0.0f, 0.0f }, false };
			sprite.vertices[1] = { { x - w, y + h - (size.y * value), z }, color, { 0.0f, 1.0f }, false };
			sprite.vertices[2] = { { x + w, y + h - (size.y * value), z }, color, { 1.0f, 1.0f }, false };
			sprite.vertices[3] = { { x + w, y + h, z }, color, { 1.0f, 0.0f }, false };
			break;
		case SliderAlignment::BOTTOM_TO_TOP:
			sprite.vertices[0] = { { x - w, y - h, z }, color, { 0.0f, 0.0f }, false };
			sprite.vertices[1] = { { x - w, y - h + (size.y * value), z }, color, { 0.0f, 1.0f }, false };
			sprite.vertices[2] = { { x + w, y - h + (size.y * value), z }, color, { 1.0f, 1.0f }, false };
			sprite.vertices[3] = { { x + w, y - h, z }, color, { 1.0f, 0.0f }, false };
			break;

		default:
			sprite.vertices[0] = { { x - w, y + h, z }, color, { 0.0f, 0.0f }, false };
			sprite.vertices[1] = { { x - w, y - h, z }, color, { 0.0f, 1.0f }, false };
			sprite.vertices[2] = { { x + w, y - h, z }, color, { 1.0f, 1.0f }, false };
			sprite.vertices[3] = { { x + w, y + h, z }, color, { 1.0f, 0.0f }, false };
			break;
	}

	//update the indices
	uint32_t index = 0;
	sprite.indices[index++] = 0;
	sprite.indices[index++] = 1;
	sprite.indices[index++] = 2;
	sprite.indices[index++] = 0;
	sprite.indices[index++] = 2;
	sprite.indices[index++] = 3;

	sprite.type = sprite.type = TransparentType::SPRITE;
	sprite.billboard = true;
	sprite.size = size / 2.0f;
	sprite.position = position;

	current_scene->transparent_objects.push_back(sprite);
}
void SpriteDraw::draw_sprite_scene(RenderScene *scene, const glm::vec3 &position, const glm::vec2 &size,
		const glm::vec3 &color, const char *texture_name, bool is_screen_space, Alignment alignment) {
	Texture t = SpriteManager::get()->get_sprite_texture(texture_name);

	TransparentObject sprite =
			default_vertex_data(position, size, (float)t.width, (float)t.height, color, is_screen_space, alignment);
	sprite.texture_name = texture_name;

	scene->transparent_objects.push_back(sprite);
}
