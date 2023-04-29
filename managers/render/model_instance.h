#ifndef SILENCE_MODEL_INSTANCE_H
#define SILENCE_MODEL_INSTANCE_H

#include "model.h"

template <typename T> struct Handle {
	uint32_t id;
};

struct ModelInstance {
	Handle<Model> model_handle;
	glm::mat4 transform;
};

#endif // SILENCE_MODEL_INSTANCE_H