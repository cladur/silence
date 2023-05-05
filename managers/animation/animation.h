#ifndef SILENCE_ANIMATION_H
#define SILENCE_ANIMATION_H

#include "bone.h"
#include <tiny_gltf.h>
class SkinnedModel;
class Joint;

struct HierarchyData {
	uint16_t position[3];
	uint16_t rotation[3];
	std::string name;
	std::vector<HierarchyData> children;
};

class Animation {
public:
	Animation() = default;

	explicit Animation(const char *animation_path);

	void process_animation(const tinygltf::Animation &animation);
	int32_t get_ticks_per_second() const;
	double get_duration() const;
	const HierarchyData &get_root_node() const;
	Bone *find_bone(const std::string &name);

private:
	const float SECONDS_TO_MS = 1000.0f;

	tinygltf::Model model;
	void read_hierarchy_data(HierarchyData &dest, const tinygltf::Node &src);
	void load_bones(const AnimNode &node, int32_t id);
	double duration_ms;
	int32_t ticks_per_second;
	std::vector<Bone> bones;
	HierarchyData root_node;
	std::unordered_map<std::string, Joint> joint_map;
	std::vector<AnimNode> temp_anim_nodes;
};

#endif //SILENCE_ANIMATION_H
