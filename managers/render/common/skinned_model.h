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
	Rig rig;

	std::vector<SkinnedMesh> meshes;
	std::unordered_map<std::string, Joint> joint_map;

private:
	Rig sort_rig_bones();
	std::vector<std::pair<std::vector<int32_t>, int32_t>> sort_children(int32_t parent, int32_t level);
	void update_parent_indices(Rig &result);

	// model data
	std::string directory;
};
#endif //SILENCE_SKINNED_MODEL_H
