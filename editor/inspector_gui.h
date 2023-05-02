#ifndef SILENCE_INSPECTOR_GUI_H
#define SILENCE_INSPECTOR_GUI_H

#include "ecs/ecs_manager.h"
#include "render/render_manager.h"
#include <imgui.h>
#include <cstdint>
class Inspector {
private:
	Entity selected_entity = 0;
	Signature selected_entity_signature;
	std::vector<int> selected_entity_components;
	std::vector<int> not_selected_entity_components;
	ImGuiTreeNodeFlags tree_flags =
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth;
	ECSManager &ecs_manager = ECSManager::get();
	RenderManager &render_manager = RenderManager::get();

	void (*show_component_functions[12])(Entity entity);
	void show_component(int signature_index);
	void show_name();
	void show_transform();
	void show_rigidbody();
	void show_gravity();
	void show_parent();
	void show_children();
	void show_modelinstance();
	void show_fmodlistener();
	void show_collidertag();
	void show_collidersphere();
	void show_collideraabb();
	void show_colliderobb();
	bool show_vec3(const char *label, glm::vec3 &vec3, float speed = 0.1f);

public:
	void show_components();
	void show_add_component();
	void set_active_entity(Entity entity);
};

#endif //SILENCE_INSPECTOR_GUI_H
