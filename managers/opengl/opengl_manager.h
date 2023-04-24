#ifndef SILENCE_OPENGL_MANAGER_H
#define SILENCE_OPENGL_MANAGER_H

#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "texture.h"

#include "camera/camera.h"

struct PrefabInstance {
	std::vector<uint32_t> mesh_ids;

	void serialize_json(nlohmann::json &j) {
		// TODO good serialization
		nlohmann::json::object_t obj;
		// obj["mesh"] = "box";
		// obj["material"] = "default_mesh";
		j.push_back(nlohmann::json::object());
		j.back()["prefab_instance"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("mesh_instance", j);
		// mesh = render_manager.get_mesh(obj["mesh"]);
		// material = render_manager.get_material(obj["material"]);
	}
};

class OpenglManager {
private:
	// std::vector<Mesh> meshes;
	// std::vector<PrefabInstance> prefab_instances;

public:
	Mesh mesh;
	Shader shader;
	Texture texture;
	static OpenglManager *get();

	void startup();
	void shutdown();
	void draw(Camera &camera);

	void update_transform(uint32_t object_id, glm::mat4 const &transform);
};

#endif // SILENCE_OPENGL_MANAGER_H