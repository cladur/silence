#include "material.h"
#include <render/transparent_elements/ui/sprite_manager.h>

#include "display/display_manager.h"

#include "camera/camera.h"

#include "render_pass.h"

#include "render/render_manager.h"
#include "render/render_scene.h"

void MaterialUnlit::startup() {
	shader.load_from_files(shader_path("unlit.vert"), shader_path("unlit.frag"));
}

void MaterialUnlit::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);
	shader.set_vec3("camPos", scene.camera_pos);
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

void MaterialPBR::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);
	shader.set_vec3("camPos", scene.camera_pos);
	shader.set_int("albedo_map", 0);
	shader.set_int("normal_map", 1);
	shader.set_int("ao_metallic_roughness_map", 2);
	shader.set_int("emissive_map", 3);

	shader.set_int("irradiance_map", 5);
	shader.set_int("prefilter_map", 6);
	shader.set_int("brdf_lut", 7);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.irradiance_map.id);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox_pass.skybox.prefilter_map.id);

	glActiveTexture(GL_TEXTURE7);
	// TODO: Use baked brdf lut instead (it's broken atm)
	glBindTexture(GL_TEXTURE_2D, scene.skybox_pass.skybox.brdf_lut_texture);

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
	shader.set_vec3("lightPositions[3]", scene.camera_pos);

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

void MaterialSkybox::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_int("environment_map", 0);
	shader.set_mat4("view", scene.view);
	shader.set_mat4("projection", scene.projection);
}

void MaterialSkybox::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialTransparent::startup() {
	shader.load_from_files(shader_path("transparent.vert"), shader_path("transparent.frag"));
}

void MaterialTransparent::bind_resources(RenderScene &scene) {
	shader.use();
	shader.set_mat4("view", scene.view);
}

void MaterialTransparent::bind_instance_resources(ModelInstance &instance, Transform &transform) {
}

void MaterialTransparent::bind_object_resources(RenderScene &scene, TransparentObject &object) {
	static Texture t;
	static int textured = 0;
	static glm::mat4 view = scene.view;
	static glm::vec2 window_size = DisplayManager::get().get_window_size();

	if (object.type == TransparentType::TEXT) {
		t = FontManager::get().fonts[object.texture_name].texture;
		shader.set_int("is_sprite", 0);

	} else if (object.type == TransparentType::SPRITE) {
		t = SpriteManager::get()->get_sprite_texture(object.texture_name);
		shader.set_int("is_sprite", 1);
	}
	textured = !object.texture_name.empty();

	if (object.vertices[0].is_screen_space) {
		shader.set_mat4("projection", glm::ortho(0.0f, window_size.x, 0.0f, window_size.y, 0.1f, 100.0f));
	} else {
		shader.set_mat4("projection", scene.projection);
	}

	shader.set_int("_texture", 0);

	shader.set_int("textured", textured);
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
}