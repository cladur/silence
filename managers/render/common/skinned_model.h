#ifndef SILENCE_SKINNED_MODEL_H
#define SILENCE_SKINNED_MODEL_H

#include "skinned_mesh.h"

struct Rig {
	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<std::string> names;
	std::vector<int64_t> parents;
	std::vector<std::vector<int32_t>> children;
};

struct Bone {
	glm::vec3 translation;
	glm::quat rotation;
	std::string name;
	std::vector<Bone> children;
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
	Bone root;

	std::vector<SkinnedMesh> meshes;
	std::unordered_map<std::string, Joint> joint_map;

private:
	void process_bone(int32_t bone_index, Bone &bone);

	Rig rig;
	// model data
	std::string directory;
};
#endif //SILENCE_SKINNED_MODEL_H
