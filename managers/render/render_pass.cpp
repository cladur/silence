#include "render_pass.h"
#include "render/material.h"
#include "render/render_manager.h"

void RenderPass::add_instance(Handle<ModelInstance> handle) {
	instance_handles.push_back(handle);
}

void RenderPass::remove_instance(Handle<ModelInstance> handle) {
	for (int i = 0; i < instance_handles.size(); i++) {
		if (instance_handles[i].id == handle.id) {
			instance_handles.erase(instance_handles.begin() + i);
			return;
		}
	}
}

void UnlitPass::startup() {
	material.startup();
}

void UnlitPass::draw() {
	RenderManager *render_manager = RenderManager::get();
	material.bind_resources();
	for (auto &handle : instance_handles) {
		ModelInstance &instance = render_manager->get_model_instance(handle);
		material.bind_instance_resources(instance);
		Model &model = render_manager->get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			material.bind_mesh_resources(mesh);
			mesh.draw();
		}
	}
}

void PBRPass::startup() {
	material.startup();
}

void PBRPass::draw() {
	RenderManager *render_manager = RenderManager::get();
	material.bind_resources();
	for (auto &handle : instance_handles) {
		ModelInstance &instance = render_manager->get_model_instance(handle);
		material.bind_instance_resources(instance);
		Model &model = render_manager->get_model(instance.model_handle);
		for (auto &mesh : model.meshes) {
			material.bind_mesh_resources(mesh);
			mesh.draw();
		}
	}
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
