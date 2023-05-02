#include "transparent_draw.h"
#include "display/display_manager.h"
#include <opengl/opengl_manager.h>
#include <opengl/transparent_elements/ui/sprite_manager.h>

const uint32_t VERTEX_COUNT = 4000;
const uint32_t INDEX_COUNT = 6000;

void TransparentDraw::startup() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, VERTEX_COUNT * sizeof(TransparentVertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)nullptr);
	// vertex color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, color));
	// vertex uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, uv));
	// is screen space
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, is_screen_space));

	glBindVertexArray(0);

	shader.load_from_files(shader_path("transparent.vert"), shader_path("transparent.frag"));
}

void TransparentDraw::draw() {
	auto manager = OpenglManager::get();
	auto d_manager = DisplayManager::get();
	static std::vector<TransparentObject> screen_space_objects;
	glm::vec3 cam_pos = manager->camera_pos;

	// transparency sorting for world-space objects
	std::sort(objects.begin(), objects.end(), [cam_pos](const TransparentObject &a, const TransparentObject &b) {
		SPDLOG_INFO("{} cam distance {}, {} cam distance {}", a.texture_name ,glm::distance(cam_pos, a.position), b.texture_name, glm::distance(cam_pos, b.position));
		return glm::distance(cam_pos, a.position) > glm::distance(cam_pos, b.position);
	});

	static glm::mat4 view = manager->view;
	static glm::mat4 proj = manager->projection;
	static glm::vec2 window_size = d_manager->get_framebuffer_size();
	static Texture t;
	static int textured = 0;

	shader.use();

	for (auto &object : objects) {
		if (object.vertices[0].is_screen_space == 1) {
			// skup drawing billboards now, we'll draw them after we're finished with all the world-space objects
			screen_space_objects.push_back(object);
			SPDLOG_INFO("screen space object pushed");
			continue;
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		if (object.type == TransparentType::TEXT) {

			// TODO text
			t = FontManager::get()->fonts[object.texture_name].texture;
			shader.set_int("is_sprite", 0);

		} else if (object.type == TransparentType::SPRITE) {

			t = SpriteManager::get()->get_sprite_texture(object.texture_name);
			shader.set_int("is_sprite", 1);
		}
		textured = !object.texture_name.empty();

		view = manager->view;
		proj = manager->projection;

		shader.set_mat4("projection", proj);
		shader.set_mat4("view", view);
		shader.set_int("textured", textured);
		shader.set_int("_texture", 0);

		if (object.billboard) {
			auto right = glm::vec3(view[0][0], view[1][0], view[2][0]);
			auto up = glm::vec3(view[0][1], view[1][1], view[2][1]);
			auto look = glm::vec3(view[0][2], view[1][2], view[2][2]);
			shader.set_vec3("camera_right", right);
			shader.set_vec3("camera_up", up);
			shader.set_vec3("camera_look", look);
			shader.set_vec2("size", object.size);
			shader.set_vec3("billboard_center", object.position);
			shader.set_int("is_billboard", 1);
		} else {
			shader.set_int("is_billboard", 0);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t.id);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	// we need to sort screen-space object separately cos their z values are in screen space and don't change depending on player movement

	std::sort(screen_space_objects.begin(), screen_space_objects.end(), [](const TransparentObject &a, const TransparentObject &b) {
		return a.vertices[0].position.z < b.vertices[0].position.z;
	});

	for (auto &object : screen_space_objects) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		if (object.type == TransparentType::TEXT) {

			// TODO text
			t = FontManager::get()->fonts[object.texture_name].texture;
			shader.set_int("is_sprite", 0);

		} else if (object.type == TransparentType::SPRITE) {

			t = SpriteManager::get()->get_sprite_texture(object.texture_name);
			shader.set_int("is_sprite", 1);
		}
		textured = !object.texture_name.empty();

		view = manager->view;
		proj = glm::ortho(0.0f, window_size.x, 0.0f, window_size.y);

		shader.set_mat4("projection", proj);
		shader.set_mat4("view", view);
		shader.set_int("textured", textured);
		shader.set_int("_texture", 0);

		shader.set_int("is_billboard", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t.id);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	objects.clear();
	screen_space_objects.clear();
}
