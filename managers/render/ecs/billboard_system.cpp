#include "billboard_system.h"
#include <ecs/world.h>
#include <managers/render/ecs/billboard_component.h>
#include <render/transparent_elements/ui_manager.h>

void BillboardSystem::startup(World &world) {

	Signature whitelist;

	whitelist.set(world.get_component_type<Billboard>());

	world.set_system_component_whitelist<BillboardSystem>(whitelist);

	UIManager::get().create_ui_scene(ui_scene_name);
	UIManager::get().activate_ui_scene(ui_scene_name);
}

void BillboardSystem::update(World &world, float dt) {
	ZoneScopedN("BillboardSystem::update");

	auto &ui = UIManager::get();

	for (auto const &entity : entities) {
		auto &billboard = world.get_component<Billboard>(entity);
		auto &transform = world.get_component<Transform>(entity);

		if (billboard.first_frame) {
			billboard.ui_name = "billboard_entity_" + std::to_string(entity);
			auto &bill = ui.add_ui_image(ui_scene_name, billboard.ui_name);
			bill.is_billboard = true;
			bill.is_screen_space = false;
			bill.display = true;
			bill.position = transform.position + billboard.position_offset;
			bill.size = billboard.scale;
			bill.texture = billboard.texture;
			bill.color = billboard.color;
			bill.use_camera_right = billboard.use_camera_right;
			bill.billboard_z_offset = billboard.billboard_z_offset;
			ui.add_as_root(ui_scene_name, billboard.ui_name);
			billboard.first_frame = false;
		} else {
			auto &bill = ui.get_ui_image(ui_scene_name, billboard.ui_name);
			bill.is_billboard = true;
			bill.is_screen_space = false;
			bill.display = true;
			bill.position = transform.position + billboard.position_offset;
			bill.size = billboard.scale;
			bill.texture = billboard.texture;
			bill.color = billboard.color;
			bill.use_camera_right = billboard.use_camera_right;
			bill.billboard_z_offset = billboard.billboard_z_offset;
		}
	}
}
