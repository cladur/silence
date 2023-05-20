#ifndef SILENCE_INSPECTOR_GUI_H
#define SILENCE_INSPECTOR_GUI_H

#include "ecs/world.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include <imgui.h>

class Inspector {
private:
	Entity selected_entity = 0;
	ImGuiTreeNodeFlags tree_flags =
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth;
	ResourceManager &resource_manager = ResourceManager::get();

	std::queue<std::pair<Entity, int>> remove_component_queue;

	void show_name();
	void show_transform();
	void show_rigidbody();
	void show_parent();
	void show_children();
	void show_skinnedmodelinstance();
	void show_animationinstance();
	void show_modelinstance();
	void show_fmodlistener();
	void show_camera();
	void show_collidertag();
	void show_statictag();
	void show_collidersphere();
	void show_collideraabb();
	void show_colliderobb();
	void show_light();
	void show_agent_data();
	void show_hacker_data();
	void show_enemy_path();
	void show_interactable();
	static bool show_vec3(const char *label, glm::vec3 &vec3, float speed = 0.1f, float reset_value = 0.0f,
			float min_value = 100.0f, float max_value = 100.0f);
	static bool show_float(const char *label, float &value, float speed = 0.1f);
	static void show_checkbox(const char *label, bool &value);
	static void show_text(const char *label, const char *value);
	static void show_vector_vec3(const char *label, std::vector<glm::vec3> &vec3);

	template <typename T> void remove_component_popup() {
		std::string popup_name = fmt::format("Remove {}", typeid(T).name());
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup(popup_name.c_str());
		}

		if (ImGui::BeginPopup(popup_name.c_str())) {
			remove_component_menu_item<T>();
			ImGui::EndPopup();
		}
	};

	template <typename T> void remove_component_menu_item() {
		if (ImGui::MenuItem("Remove component")) {
			// Type to component id
			remove_component_queue.emplace(selected_entity, world->get_component_id<T>());
		}
	}

	template <> void remove_component_menu_item<Transform>() {
		if (ImGui::MenuItem("Remove component")) {
			SPDLOG_ERROR("Cannot remove Transform component (Jan fix pls)");
		}
	}

public:
	World *world;

	void show_components();
	void show_add_component();
	void set_active_entity(Entity entity);
	static void show_text(const char *label, int value);
};

#endif //SILENCE_INSPECTOR_GUI_H
