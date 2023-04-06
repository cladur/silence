#ifndef SILENCE_MESHINSTANCE_H
#define SILENCE_MESHINSTANCE_H

#include "render/render_manager.h"
#include "render/vk_mesh.h"
#include "types.h"

extern RenderManager render_manager;

struct MeshInstance {
	Mesh *mesh;
	Material *material;

	void serialize_json(nlohmann::json &j) {
		// TODO good serialization
		nlohmann::json::object_t obj;
		obj["mesh"] = "box";
		obj["material"] = "default_mesh";
		j.push_back(nlohmann::json::object());
		j.back()["mesh_instance"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("mesh_instance", j);
		mesh = render_manager.get_mesh(obj["mesh"]);
		material = render_manager.get_material(obj["material"]);
	}
};

#endif //SILENCE_MESHINSTANCE_H
