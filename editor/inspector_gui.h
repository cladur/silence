#ifndef SILENCE_INSPECTOR_GUI_H
#define SILENCE_INSPECTOR_GUI_H

#include "ecs/ecs_manager.h"
#include "render/render_manager.h"
#include <imgui.h>
#include <cstdint>
class Inspector {
private:
	ImGuiTreeNodeFlags tree_flags =
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth;
	ECSManager &ecs_manager = ECSManager::get();
	RenderManager &render_manager = RenderManager::get();

	void (*show_component_functions[12])(Entity entity);
	void show_component(Entity entity, int signature_index);
	void show_name(Entity entity);
	void show_transform(Entity entity);
	void show_rigidbody(Entity entity);
	void show_gravity(Entity entity);
	void show_parent(Entity entity);
	void show_children(Entity entity);
	void show_modelinstance(Entity entity);
	void show_fmodlistener(Entity entity);
	void show_collidertag(Entity entity);
	void show_collidersphere(Entity entity);
	void show_collideraabb(Entity entity);
	void show_colliderobb(Entity entity);
	bool show_vec3(const char *label, glm::vec3 &vec3, float speed = 0.1f);

public:
	void show_components(Entity entity);
};

#endif //SILENCE_INSPECTOR_GUI_H
