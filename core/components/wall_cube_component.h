#ifndef SILENCE_WALL_CUBE_COMPONENT_H
#define SILENCE_WALL_CUBE_COMPONENT_H

#include "managers/render/common/model.h"
#include "managers/resource/resource_manager.h"
#include <nlohmann/json.hpp>
struct WallCube {
	Entity faces_parent = 0;
	Handle<Model> model_handle;
	bool scale_uv = true;

	WallCube() {
		ResourceManager &resource_manager = ResourceManager::get();
		model_handle = resource_manager.load_model(asset_path("items/wooden_box/wooden_box.mdl").c_str());
	}

	void serialize_json(nlohmann::json &serialized_scene) {
		ResourceManager &resource_manager = ResourceManager::get();
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["faces_parent"] = faces_parent;
		serialized_component["model_name"] = resource_manager.get_model_name(model_handle);
		serialized_component["scale_uv"] = scale_uv;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "WallCube";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		faces_parent = serialized_component["faces_parent"];

		if (serialized_component.contains("model_name")) {
			std::string model_name = serialized_component["model_name"];
			ResourceManager &resource_manager = ResourceManager::get();
			model_handle = resource_manager.load_model(asset_path(model_name).c_str());
		}

		if (serialized_component.contains("scale_uv")) {
			scale_uv = serialized_component["scale_uv"];
		} else {
			scale_uv = true;
		}
	}
};

#endif //SILENCE_WALL_CUBE_COMPONENT_H
