#ifndef SILENCE_SKINNED_MODEL_INSTANCE_H
#define SILENCE_SKINNED_MODEL_INSTANCE_H

#include "model_instance.h"
#include "render/common/material.h"
#include "render/common/skinned_model.h"

struct Pose {
	std::vector<glm::mat4> matrices;
};

class SkinnedModelInstance {
public:
	Handle<SkinnedModel> model_handle;
	MaterialType material_type = MaterialType::Default;
	Pose current_pose;
	GLuint skinning_buffer;
	bool in_shadow_pass = true;

	SkinnedModelInstance();
	void release();
	explicit SkinnedModelInstance(const char *path, MaterialType material_type = MaterialType::Default);

	void serialize_json(nlohmann::json &j);
	void deserialize_json(nlohmann::json &j);
};

#endif //SILENCE_SKINNED_MODEL_INSTANCE_H
