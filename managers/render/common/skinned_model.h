#ifndef SILENCE_SKINNED_MODEL_H
#define SILENCE_SKINNED_MODEL_H

#include "skinned_mesh.h"

struct Rig {
	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<std::string> names;
	std::vector<int64_t> parents;
};

struct Joint {
	glm::vec3 translation;
	glm::quat rotation;
	int32_t id;
};

class SkinnedModel {
public:
	void load_from_asset(const char *path);

	std::string name;
	glm::mat4 root{};

	std::vector<SkinnedMesh> meshes;
	Rig rig;
	std::unordered_map<std::string, Joint> joint_map;

private:
	// model data
	std::string directory;
};
#endif //SILENCE_SKINNED_MODEL_H
