#ifndef SILENCE_RENDER_PASS_H
#define SILENCE_RENDER_PASS_H

#include "material.h"
#include "model.h"
#include "model_instance.h"

struct InstanceInfo {
	bool in_forward_pass;
	bool in_shadow_pass;
};

class RenderPass {
public:
	std::vector<Handle<ModelInstance>> instance_handles;

	void add_instance(Handle<ModelInstance> handle);
	void remove_instance(Handle<ModelInstance> handle);

	virtual void startup() = 0;
	virtual void draw() = 0;
};

class UnlitPass : public RenderPass {
public:
	MaterialUnlit material;
	void startup() override;
	void draw() override;
};

#endif // SILENCE_RENDER_PASS_H