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
	shader.set_int("ao_map", 1);
	shader.set_int("normal_map", 2);
	shader.set_int("metallic_roughness_map", 3);
	shader.set_int("emissive_map", 4);
}

void MaterialUnlit::bind_instance_resources(ModelInstance &instance) {
	shader.set_mat4("model", instance.transform);
}