#ifndef SILENCE_MODEL_INSTANCE_H
#define SILENCE_MODEL_INSTANCE_H

#include "render/common/material.h"
#include "render/common/model.h"

template <typename T> struct Handle {
	uint32_t id;
};

struct ModelInstance {
	Handle<Model> model_handle;
	MaterialType material_type = MaterialType::Default;
	bool in_shadow_pass = true;

	ModelInstance();
	explicit ModelInstance(const char *path, MaterialType material_type = MaterialType::Default);

	void serialize_json(nlohmann::json &j);
	void deserialize_json(nlohmann::json &j);
};

#endif // SILENCE_MODEL_INSTANCE_H