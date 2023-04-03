#ifndef SILENCE_MESHINSTANCE_H
#define SILENCE_MESHINSTANCE_H

#include "render/render_manager.h"
#include "render/vk_mesh.h"
struct MeshInstance {
	Mesh *mesh;
	Material *material;

	void serialize(nlohmann::json &j) {
		// TODO good serialization
		nlohmann::json::object_t obj;
		obj["mesh"] = "Mesh";
		obj["material"] = "Material";
		j.push_back(nlohmann::json::object());
		j.back()["mesh_instance"] = obj;
	}
};

#endif //SILENCE_MESHINSTANCE_H
