#ifndef SILENCE_MODEL_INSTANCE_H
#define SILENCE_MODEL_INSTANCE_H

#include "render/common/material.h"
#include "render/common/model.h"
#include "resource/resource_manager.h"

struct ModelInstance {
	Handle<Model> model_handle;
	MaterialType material_type = MaterialType::Default;
	bool in_shadow_pass = true;
	bool scale_uv_with_transform = false;

	glm::vec2 uv_scale = glm::vec2(1.0f, 1.0f);

	ModelInstance();
	explicit ModelInstance(
			const char *path, MaterialType material_type = MaterialType::Default, bool scale_uv_with_transform = true);

	void serialize_json(nlohmann::json &j);
	void deserialize_json(nlohmann::json &j);
};

#endif // SILENCE_MODEL_INSTANCE_H