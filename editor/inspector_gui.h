#ifndef SILENCE_INSPECTOR_GUI_H
#define SILENCE_INSPECTOR_GUI_H

#include "ecs/ecs_manager.h"
#include "render/render_manager.h"
#include <imgui.h>
#include <cstdint>
#include <typeindex>
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

	std::unordered_map<int, std::function<void()>> show_component_map;
	std::unordered_map<std::type_index, std::function<void()>> type_to_show_functions_map;

	static void show_component(int signature_index);
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
	static bool show_vec3(const char *label, glm::vec3 &vec3, float speed = 0.1f);
	static bool show_float(const char *label, float &value, float speed = 0.1f);
	static void show_checkbox(const char *label, bool &value);
	static void show_text(const char *label, const char *value);

public:
	Inspector();
	static Inspector &get();
	void show_components();
	void show_add_component();
	void refresh_entity();
	void set_active_entity(Entity entity);
	static void add_mapping(int signature_index, std::function<void()> func);
	template <typename T> static void show_component() {
		Inspector &inspector = Inspector::get();
		auto map = inspector.type_to_show_functions_map;
		auto it = map.find(typeid(T));
		if (it != map.end()) {
			it->second();
		} else {
			SPDLOG_ERROR("No show function for component {}", typeid(T).name());
		}
	};
	static void show_text(const char *label, int value);
};

#endif //SILENCE_INSPECTOR_GUI_H
