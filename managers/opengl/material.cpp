#include "material.h"

#include "display/display_manager.h"

#include "camera/camera.h"

#include "render_pass.h"

#include "opengl_manager.h"

void MaterialUnlit::startup() {
	shader.load_from_files(shader_path("unlit.vert"), shader_path("unlit.frag"));
}

void MaterialUnlit::bind_resources() {
	OpenglManager *opengl_manager = OpenglManager::get();
	shader.use();
	shader.set_mat4("view", opengl_manager->view);
	shader.set_mat4("projection", opengl_manager->projection);
	shader.set_vec3("camPos", opengl_manager->camera_pos);
	shader.set_int("albedo_map", 0);
}

void MaterialUnlit::bind_instance_resources(ModelInstance &instance) {
	shader.set_mat4("model", instance.transform);
}

void MaterialPBR::startup() {
	shader.load_from_files(shader_path("pbr.vert"), shader_path("pbr.frag"));
}

void MaterialPBR::bind_resources() {
	OpenglManager *opengl_manager = OpenglManager::get();
	shader.use();
	shader.set_mat4("view", opengl_manager->view);
	shader.set_mat4("projection", opengl_manager->projection);
	shader.set_vec3("camPos", opengl_manager->camera_pos);
	shader.set_int("albedo_map", 0);
	shader.set_int("ao_map", 1);
	shader.set_int("normal_map", 2);
	shader.set_int("metallic_roughness_map", 3);
	shader.set_int("emissive_map", 4);

	shader.set_int("irradiance_map", 5);
	shader.set_int("prefilter_map", 6);
	shader.set_int("brdf_lut", 7);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, opengl_manager->skybox_pass.skybox.irradiance_map.id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, opengl_manager->skybox_pass.skybox.prefilter_map.id);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, opengl_manager->skybox_pass.skybox.brdf_lut.id);

	shader.set_vec3("lightPositions[0]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightPositions[1]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightPositions[2]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightPositions[3]", opengl_manager->camera_pos);

	shader.set_vec3("lightColors[0]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightColors[1]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightColors[2]", glm::vec3(0.0f, 0.0f, 0.0f));
	shader.set_vec3("lightColors[3]", glm::vec3(1.0f, 1.0f, 1.0f));
}

void MaterialPBR::bind_instance_resources(ModelInstance &instance) {
	shader.set_mat4("model", instance.transform);
}

void MaterialSkybox::startup() {
	shader.load_from_files(shader_path("cubemap.vert"), shader_path("skybox.frag"));
}

void MaterialSkybox::bind_resources() {
	OpenglManager *opengl_manager = OpenglManager::get();
	shader.use();
	shader.set_int("environment_map", 0);
	shader.set_mat4("view", opengl_manager->view);
	shader.set_mat4("projection", opengl_manager->projection);
}

void MaterialSkybox::bind_instance_resources(ModelInstance &instance) {
}