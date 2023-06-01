#include "taggable_system.h"
#include <render/transparent_elements/ui_manager.h>
void TaggableSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Taggable>());

	world.set_system_component_whitelist<TaggableSystem>(whitelist);

	ui_name = "taggable_ui";
	tag_prefix = "taggable_entity_";

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
		}
	}
}
