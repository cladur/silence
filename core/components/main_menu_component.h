#ifndef SILENCE_MAIN_MENU_COMPONENT_H
#define SILENCE_MAIN_MENU_COMPONENT_H

#include "render/transparent_elements/ui/ui_elements/ui_anchor.h"
#include "render/transparent_elements/ui/ui_elements/ui_button.h"
#include "render/transparent_elements/ui/ui_elements/ui_image.h"
#include "render/transparent_elements/ui/ui_elements/ui_text.h"

struct MainMenu {

	UIImage billboard_test;

	UIAnchor *root;
	UIButton *play_button;
	UIButton *options_button;
	UIButton *credits_button;

	UIAnchor *title_root;
	UIText *title_text;

	UIAnchor *options_root;
	UIText *options_text;
	UIText *master_volume;
	std::array<UIImage *, 10> volume_meter;
	UIButton *plus_button;
	UIButton *minus_button;

	UIAnchor *back_button_root;
	UIButton *back_button;

	UIButton *quit_button;

	FMOD::Studio::Bus *master_bus = nullptr;

	bool first_frame = true;
	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "MainMenu";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
	}
};

#endif //SILENCE_MAIN_MENU_COMPONENT_H