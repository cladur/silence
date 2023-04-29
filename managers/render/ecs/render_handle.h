#ifndef SILENCE_RENDER_HANDLE_H
#define SILENCE_RENDER_HANDLE_H

#include "model_instance.h"

struct RenderHandle {
	Handle<ModelInstance> handle;

	void serialize_json(nlohmann::json &j) {
		// TODO good serialization
		nlohmann::json::object_t obj;
		// obj["mesh"] = "box";
		// obj["material"] = "default_mesh";
		j.push_back(nlohmann::json::object());
		j.back()["prefab_instance"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("render_handle", j);
		// mesh = render_manager.get_mesh(obj["mesh"]);
		// material = render_manager.get_material(obj["material"]);
	}
};

#endif