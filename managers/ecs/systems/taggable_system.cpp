#include "taggable_system.h"
#include <render/transparent_elements/ui_manager.h>
void TaggableSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Taggable>());

	world.set_system_component_whitelist<TaggableSystem>(whitelist);

	ui_name = "taggable_ui";
	tag_prefix = "taggable_entity_";

	auto &rm = ResourceManager::get();

	tag_texture = rm.load_texture(asset_path("tag_mono.ktx2").c_str());

	auto &ui = UIManager::get();
	ui.create_ui_scene(ui_name);
	ui.activate_ui_scene(ui_name);
}

void TaggableSystem::update(World &world, float dt) {
	auto &rm = ResourceManager::get();
	auto &ui = UIManager::get();

	for (const Entity entity : entities) {
		auto &tag = world.get_component<Taggable>(entity);
		auto &transform = world.get_component<Transform>(entity);

		if (tag.fist_frame) {
			std::string ui_tag_name = tag_prefix + std::to_string(entity);
			auto &sprite = ui.add_ui_image(ui_name, ui_tag_name);
			sprite.is_screen_space = false;
			sprite.is_billboard = true;
			sprite.position = transform.get_global_position() + tag.tag_position;
			sprite.texture = tag_texture;
			sprite.size = glm::vec2(0.25f);
			sprite.display = true;

			ui.add_as_root(ui_name, ui_tag_name);

			tag.fist_frame = false;
		}

		glm::vec4 color = glm::vec4(0.2f);

		auto &sprite = ui.get_ui_image(ui_name, tag_prefix + std::to_string(entity));

		sprite.position = transform.get_global_position() + tag.tag_position;
		if (tag.tagged) {
			// different types of tags, for now colors, later maybe different sprites
			if (world.has_component<Interactable>(entity)) {
				color = glm::vec4(0.1f, 1.0f, 0.1f, 0.9f);
			} else if (world.has_component<EnemyData>(entity)) {
				color = glm::vec4(1.0f, 0.0f, 0.0f, 0.9f);
			} else {
				color = glm::vec4(0.4f, 0.6f, 1.0f, 0.9f);
			}

		} else if (tag.highlight){
			tag.highlight = false;
			color = glm::vec4(0.2f, 0.2f, 0.9f, 0.4f);
		}
		sprite.color = color;
	}
}
