#include "taggable_system.h"
#include <audio/audio_manager.h>
#include <render/transparent_elements/ui_manager.h>

void TaggableSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Taggable>());

	world.set_system_component_whitelist<TaggableSystem>(whitelist);

	ui_name = "taggable_ui";
	tag_prefix = "taggable_entity_";

	on_tagged = EventReference("SFX/tag"); 

	auto &rm = ResourceManager::get();

	tag_texture = rm.load_texture(asset_path("tag_mono.ktx2").c_str());

	non_tagged_color = glm::vec4(0.2f);
	enemy_color = glm::vec4(1.0f, 0.0f, 0.0f, 0.9f);
	interactive_color = glm::vec4(0.1f, 1.0f, 0.1f, 0.9f);
	default_color = glm::vec4(0.4f, 0.6f, 1.0f, 0.9f);

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
			glm::vec3 glob_pos = tag.tag_position.x * transform.get_right() + tag.tag_position.y * transform.get_up() +
								 tag.tag_position.z * transform.get_forward();
			sprite.position = transform.get_global_position() + glob_pos;
			sprite.texture = tag_texture;
			sprite.size = glm::vec2(0.25f);
			sprite.display = true;

			ui.add_as_root(ui_name, ui_tag_name);

			tag.fist_frame = false;
		}

		glm::vec4 color = glm::vec4(0.2f);

		auto &sprite = ui.get_ui_image(ui_name, tag_prefix + std::to_string(entity));

		glm::vec3 glob_pos = tag.tag_position.x * transform.get_right() + tag.tag_position.y * transform.get_up() +
				tag.tag_position.z * transform.get_forward();

		sprite.position = transform.get_global_position() + glob_pos;
		if (!tag.tagged) {
			float x = tag.tag_timer / tag.time_to_tag;
			if (tag.tagging) {
				tag.tag_timer += dt;
				if (tag.tag_timer >= tag.time_to_tag) {
					AudioManager::get().play_one_shot_2d(on_tagged);
					tag.tagged = true;
				}
			} else {
				tag.tag_timer -= dt;
				tag.tag_timer = glm::max(tag.tag_timer, 0.0f);
			}

			if (world.has_component<Interactable>(entity)) {
				color = glm::mix(non_tagged_color, interactive_color, x);
			} else if (world.has_component<EnemyData>(entity)) {
				color = glm::mix(non_tagged_color, enemy_color, x);
			} else {
				color = glm::mix(non_tagged_color, default_color, x);
			}

			sprite.color = color;
			tag.tagging = false;
		}


	}
}
