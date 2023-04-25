#include "material.h"

#include "display/display_manager.h"

#include "camera/camera.h"

#include "render_pass.h"

extern Camera camera;

AutoCVarFloat cvar_draw_distance("render.draw_distance", "Distance cull", 5000);

void MaterialUnlit::startup() {
	shader.load_from_files(shader_path("unlit.vert"), shader_path("unlit.frag"));
}

void MaterialUnlit::bind_resources() {
	DisplayManager *display_manager = DisplayManager::get();
	glm::vec2 window_extent = display_manager->get_framebuffer_size();

	shader.use();
	shader.set_mat4("view", camera.get_view_matrix());
	glm::mat4 projection =
			glm::perspective(glm::radians(70.0f), window_extent.x / window_extent.y, 0.1f, cvar_draw_distance.get());
	shader.set_mat4("projection", projection);
	shader.set_mat4("model", glm::mat4(1.0f));
	shader.set_vec3("camPos", camera.get_position());
	shader.set_int("albedo_map", 0);
	shader.set_int("ao_map", 1);
	shader.set_int("normal_map", 2);
	shader.set_int("metallic_roughness_map", 3);
	shader.set_int("emissive_map", 4);
}

void MaterialUnlit::bind_instance_resources(ModelInstance &instance) {
	shader.set_mat4("model", instance.transform);
}
