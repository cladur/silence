#include "render_pass.h"
#include "opengl/material.h"
#include "opengl/opengl_manager.h"

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
	OpenglManager *opengl_manager = OpenglManager::get();
	material.bind_resources();
	for (auto &handle : instance_handles) {
		ModelInstance &instance = opengl_manager->get_model_instance(handle);
		material.bind_instance_resources(instance);
		Model &model = opengl_manager->get_model(instance.model_handle);
		model.draw(MATERIAL_TYPE_UNLIT);
	}
}

void PBRPass::startup() {
	material.startup();
}

void PBRPass::draw() {
	OpenglManager *opengl_manager = OpenglManager::get();
	material.bind_resources();
	for (auto &handle : instance_handles) {
		ModelInstance &instance = opengl_manager->get_model_instance(handle);
		material.bind_instance_resources(instance);
		Model &model = opengl_manager->get_model(instance.model_handle);
		model.draw(MATERIAL_TYPE_PBR);
	}
}

void SkyboxPass::startup() {
	material.startup();
	skybox.startup();
	skybox.load_from_directory(asset_path("cubemaps/night_sky"));
}

void SkyboxPass::draw() {
	OpenglManager *opengl_manager = OpenglManager::get();
	material.bind_resources();
	skybox.draw();
}
