#include "render_pass.h"
#include "material.h"
#include "render/common/framebuffer.h"
#include "render/common/utils.h"
#include "render/render_manager.h"

void PBRPass::startup() {
	material.startup();
}

void PBRPass::draw(RenderScene &scene) {
	RenderManager &render_manager = RenderManager::get();
	material.bind_resources(scene);
	utils::render_quad();
}

void SkyboxPass::startup() {
	material.startup();
	skybox.startup();
	skybox.load_from_directory(asset_path("cubemaps/venice_sunset"));
}

void SkyboxPass::draw(RenderScene &scene) {
	RenderManager &render_manager = RenderManager::get();
	material.bind_resources(scene);
	skybox.draw();
}

const uint32_t VERTEX_COUNT = 4000;
const uint32_t INDEX_COUNT = 6000;

void TransparentPass::startup() {
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
	glVertexAttribPointer(
			1, 3, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, color));
	// vertex uv
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, uv));
	// is screen space
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(
			3, 1, GL_INT, sizeof(TransparentVertex), (void *)offsetof(TransparentVertex, is_screen_space));

	glBindVertexArray(0);
	material.startup();
}

void GBufferPass::startup() {
	material.startup();
}

void GBufferPass::draw(RenderScene &scene) {
	RenderManager &render_manager = RenderManager::get();
	material.bind_resources(scene);
	for (auto &cmd : draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		Model &model = render_manager.get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			if (mesh.fc_bounding_sphere.is_on_frustum(scene.frustum, transform, scene)) {
				material.bind_mesh_resources(mesh);
				mesh.draw();
			}
		}
	}
	draw_commands.clear();
}

void TransparentPass::draw(RenderScene &scene) {
	RenderManager &render_manager = RenderManager::get();
	static std::vector<TransparentObject> screen_space_objects;
	glm::vec3 cam_pos = scene.camera_pos;

	// transparency sorting for world-space objects
	std::sort(scene.transparent_objects.begin(), scene.transparent_objects.end(),
			[cam_pos](const TransparentObject &a, const TransparentObject &b) {
				return glm::distance(cam_pos, a.position) > glm::distance(cam_pos, b.position);
			});

	material.bind_resources(scene);

	for (auto &object : scene.transparent_objects) {
		if (object.vertices[0].is_screen_space) {
			screen_space_objects.push_back(object);
			continue;
		}
		material.bind_object_resources(scene, object);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
	}

	// transparency sorting for screen-space objects
	std::sort(screen_space_objects.begin(), screen_space_objects.end(),
			[](const TransparentObject &a, const TransparentObject &b) {
				return a.vertices[0].position.z < b.vertices[0].position.z;
			});

	for (auto &object : screen_space_objects) {
		material.bind_object_resources(scene, object);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, object.vertices.size() * sizeof(TransparentVertex), &object.vertices[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, object.indices.size() * sizeof(uint32_t), &object.indices[0]);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, object.indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);

	screen_space_objects.clear();
	scene.transparent_objects.clear();
}
