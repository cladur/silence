#include "render_pass.h"
#include "material.h"
#include "render/common/framebuffer.h"
#include "render/common/utils.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include <tracy/tracy.hpp>

AutoCVarFloat cvar_blur_radius("render.blur_radius", "blur radius", 0.001f, CVarFlags::EditFloatDrag);
AutoCVarInt cvar_use_bloom("render.use_bloom", "use bloom", 1, CVarFlags::EditCheckbox);
AutoCVarFloat cvar_bloom_strength("render.bloom_strength", "bloom strength", 0.04f, CVarFlags::EditFloatDrag);

AutoCVarFloat cvar_gamma("render.gamma", "gamma", 2.2f, CVarFlags::EditFloatDrag);

void PBRPass::startup() {
	material.startup();
}

void PBRPass::draw(RenderScene &scene) {
	ZoneScopedN("PBRPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	utils::render_quad();
}

void LightPass::startup() {
	material.startup();
}

void LightPass::draw(RenderScene &scene) {
	ZoneScopedN("LightPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	for (auto &cmd : scene.light_draw_commands) {
		Light &light = *cmd.light;
		Transform &transform = *cmd.transform;
		material.bind_light_resources(light, transform);
		utils::render_sphere();
	}
}

void AOPass::startup() {
	material.startup();
}

void AOPass::draw(RenderScene &scene) {
	ZoneScopedN("AOPass::draw");
	RenderManager &render_manager = RenderManager::get();
	material.bind_resources(scene);
	utils::render_quad();
}

void AOBlurPass::startup() {
	material.startup();
}

void AOBlurPass::draw(RenderScene &scene) {
	ZoneScopedN("AOBlurPass::draw");
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
	ZoneScopedN("SkyboxPass::draw");
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
	ZoneScopedN("GBufferPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	for (auto &cmd : scene.draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		Model &model = resource_manager.get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			if (mesh.fc_bounding_sphere.is_on_frustum(scene.frustum, transform, scene)) {
				material.bind_mesh_resources(mesh);
				mesh.draw();
			}
		}
	}

#ifdef WIN32
	material.bind_skinned_resources(scene);
	for (auto &cmd : scene.skinned_draw_commands) {
		SkinnedModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		SkinnedModel &model = resource_manager.get_skinned_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			material.bind_mesh_resources(mesh);
			mesh.draw();
		}
	}
#endif
}

void TransparentPass::draw(RenderScene &scene) {
	ZoneScopedNC("TransparentPass::draw", 0xad074f);
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

void CombinationPass::startup() {
	material.startup();
}

void CombinationPass::draw(RenderScene &scene) {
	ZoneScopedN("CombinationPass::draw");
	material.bind_resources(scene);
	utils::render_quad();
}

void BloomPass::startup() {
	material.startup();
}

void BloomPass::draw(RenderScene &scene) {
	ZoneScopedN("BloomPass::draw");
	std::vector<BloomMip> &mips = scene.bloom_buffer.mips;

	// DOWNSAMPLING
	material.downsample.use();
	material.downsample.set_int("srcTexture", 0);
	material.downsample.set_vec2("srcResolution", scene.render_extent.x, scene.render_extent.y);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.combination_buffer.texture_id);

	for (auto &mip : mips) {
		glViewport(0, 0, mip.size.x, mip.size.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture_id, 0);
		utils::render_quad();

		material.downsample.set_vec2("srcResolution", mip.size);
		glBindTexture(GL_TEXTURE_2D, mip.texture_id);
	}

	// UPSAMPLING
	material.bloom.use();
	material.bloom.set_int("srcTexture", 0);
	material.bloom.set_float("filterRadius", cvar_blur_radius.get());

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	for (int i = mips.size() - 1; i > 0; i--) {
		auto &mip = mips[i];
		auto &next_mip = mips[i - 1];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mip.texture_id);

		glViewport(0, 0, next_mip.size.x, next_mip.size.y);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next_mip.texture_id, 0);
		utils::render_quad();
	}
	glDisable(GL_BLEND);

	scene.render_framebuffer.bind();
	glViewport(0, 0, scene.render_extent.x, scene.render_extent.y);

	// combine shader
	material.shader.use();
	material.shader.set_int("scene", 0);
	material.shader.set_int("bloom_tex", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.combination_buffer.texture_id);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, mips[0].texture_id);
	material.shader.set_int("use_bloom", cvar_use_bloom.get());
	material.shader.set_float("bloom_strength", cvar_bloom_strength.get());
	material.shader.set_float("gamma", cvar_gamma.get());
	utils::render_quad();
}

void MousePickPass::startup() {
	material.startup();
}

void MousePickPass::draw(RenderScene &scene) {
	ZoneScopedN("MousePickPass::draw");
	ResourceManager &resource_manager = ResourceManager::get();
	material.bind_resources(scene);
	for (auto &cmd : scene.draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		Entity entity = cmd.entity;
		material.bind_instance_resources(instance, transform, entity);
		Model &model = resource_manager.get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			if (mesh.fc_bounding_sphere.is_on_frustum(scene.frustum, transform, scene)) {
				mesh.draw();
			}
		}
	}

#ifdef WIN32
	for (auto &cmd : scene.skinned_draw_commands) {
		SkinnedModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		Entity entity = cmd.entity;
		material.bind_instance_resources(instance, transform, entity);
		SkinnedModel &model = resource_manager.get_skinned_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			mesh.draw();
		}
	}
#endif
}