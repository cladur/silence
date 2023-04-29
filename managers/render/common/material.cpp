#include "material.h"

#include "display/display_manager.h"

#include "camera/camera.h"

#include "render_pass.h"

#include "managers/render/render_manager.h"

void MaterialUnlit::startup() {
	shader.load_from_files(shader_path("unlit.vert"), shader_path("unlit.frag"));
}

void MaterialUnlit::bind_resources() {
	RenderManager &render_manager = RenderManager::get();
	shader.use();
	shader.set_mat4("view", render_manager.view);
	shader.set_mat4("projection", render_manager.projection);
	shader.set_vec3("camPos", render_manager.camera_pos);
	shader.set_int("albedo_map", 0);
}

void MaterialUnlit::bind_instance_resources(ModelInstance &instance, Transform &transform) {
	shader.set_mat4("model", transform.get_global_model_matrix());
}

void MaterialUnlit::bind_mesh_resources(Mesh &mesh) {
	if (mesh.textures_present[0]) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh.textures[0].id);
	}
}

void MaterialPBR::startup() {
	shader.load_from_files(shader_path("pbr.vert"), shader_path("pbr.frag"));
}

void MaterialPBR::bind_resources() {
	RenderManager &render_manager = RenderManager::get();
	shader.use();
	shader.set_mat4("view", render_manager.view);
	shader.set_mat4("projection", render_manager.projection);
	shader.set_vec3("camPos", render_manager.camera_pos);
	shader.set_int("albedo_map", 0);
	shader.set_int("normal_map", 1);
	shader.set_int("ao_metallic_roughness_map", 2);
	shader.set_int("emissive_map", 3);

	shader.set_int("irradiance_map", 5);
	shader.set_int("prefilter_map", 6);
	shader.set_int("brdf_lut", 7);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, render_manager.skybox_pass.skybox.irradiance_map.id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, render_manager.skybox_pass.skybox.prefilter_map.id);

	glActiveTexture(GL_TEXTURE7);
	// TODO: Use baked brdf lut instead (it's broken atm)
	glBindTexture(GL_TEXTURE_2D, render_manager.skybox_pass.skybox.brdf_lut_texture);

	//	ImGui::Begin("PBR");
	//	static bool baked = false;
	//	if (ImGui::Button("Flip")) {
	//		baked = !baked;
	//	}
	//	if (baked) {
	//		ImGui::Text("Baked");
	//		glBindTexture(GL_TEXTURE_2D, render_manager->skybox_pass.skybox.brdf_lut.id);
	//	} else {
	//		ImGui::Text("Not baked");
	//		glBindTexture(GL_TEXTURE_2D, render_manager->skybox_pass.skybox.brdf_lut_texture);
	//	}
	//	ImGui::End();

	shader.set_vec3("lightPositions[0]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightPositions[1]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightPositions[2]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightPositions[3]", render_manager.camera_pos);

	shader.set_vec3("lightColors[0]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightColors[1]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightColors[2]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightColors[3]", glm::vec3(1.0f, 1.0f, 1.0f));
}

void MaterialPBR::bind_instance_resources(ModelInstance &instance, Transform &transform) {
	shader.set_mat4("model", transform.get_global_model_matrix());
}

void MaterialPBR::bind_mesh_resources(Mesh &mesh) {
	for (int i = 0; i < mesh.textures.size(); i++) {
		if (mesh.textures_present[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
		}
	}
	// We flip the bools here, to force the update of uniforms on the first mesh
	static bool is_ao_map_set = !mesh.has_ao_map;
	static bool is_emissive_map_set = !mesh.textures_present[3];
	if (is_ao_map_set != mesh.has_ao_map) {
		shader.set_bool("has_ao_map", mesh.has_ao_map);
		is_ao_map_set = mesh.has_ao_map;
	}
	if (is_emissive_map_set != mesh.textures_present[3]) {
		shader.set_bool("has_emissive_map", mesh.textures_present[3]);
		is_emissive_map_set = mesh.textures_present[3];
	}
}

void MaterialSkybox::startup() {
	shader.load_from_files(shader_path("cubemap.vert"), shader_path("skybox.frag"));
}

void MaterialSkybox::bind_resources() {
	RenderManager &render_manager = RenderManager::get();
	shader.use();
	shader.set_int("environment_map", 0);
	shader.set_mat4("view", render_manager.view);
	shader.set_mat4("projection", render_manager.projection);
}

void MaterialSkybox::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}