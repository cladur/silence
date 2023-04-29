#include "render_pass.h"
#include "material.h"
#include "render/render_manager.h"

void UnlitPass::startup() {
	material.startup();
}

void UnlitPass::draw() {
	RenderManager *render_manager = RenderManager::get();
	material.bind_resources();
	for (auto &cmd : draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		Model &model = render_manager->get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			material.bind_mesh_resources(mesh);
			mesh.draw();
		}
	}
	draw_commands.clear();
}

void PBRPass::startup() {
	material.startup();
}

void PBRPass::draw() {
	RenderManager *render_manager = RenderManager::get();
	material.bind_resources();
	for (auto &cmd : draw_commands) {
		ModelInstance &instance = *cmd.model_instance;
		Transform &transform = *cmd.transform;
		material.bind_instance_resources(instance, transform);
		Model &model = render_manager->get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			material.bind_mesh_resources(mesh);
			mesh.draw();
		}
	}
	draw_commands.clear();
}

void SkyboxPass::startup() {
	material.startup();
	skybox.startup();
	skybox.load_from_directory(asset_path("cubemaps/venice_sunset"));
}

void SkyboxPass::draw() {
	RenderManager *render_manager = RenderManager::get();
	material.bind_resources();
	skybox.draw();
}
