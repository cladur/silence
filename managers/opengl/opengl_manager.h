#ifndef SILENCE_OPENGL_MANAGER_H
#define SILENCE_OPENGL_MANAGER_H

#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "texture.h"

#include "camera/camera.h"

struct ModelInstance {
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
	std::unordered_map<std::string, Texture> textures;
	std::unordered_map<std::string, Mesh> meshes;
	std::unordered_map<std::string, Model> models;
	std::vector<ModelInstance> model_instances;

public:
	Model model;
	Shader shader;
	static OpenglManager *get();

	void startup();
	void shutdown();
	void draw(Camera &camera);

	void load_mesh(const char *path);
	void load_model(const char *path);
	void load_texture(const char *path);

	void update_transform(uint32_t object_id, glm::mat4 const &transform);
};

#endif // SILENCE_OPENGL_MANAGER_H