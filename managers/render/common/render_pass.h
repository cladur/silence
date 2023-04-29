#ifndef SILENCE_RENDER_PASS_H
#define SILENCE_RENDER_PASS_H

#include "components/transform_component.h"
#include "managers/render/ecs/model_instance.h"
#include "material.h"
#include "model.h"
#include "skybox.h"

struct DrawCommand {
	ModelInstance *model_instance;
	Transform *transform;
};

class RenderPass {
public:
	std::vector<DrawCommand> draw_commands;

	virtual void startup() = 0;
	virtual void draw() = 0;
};

class UnlitPass : public RenderPass {
public:
	MaterialUnlit material;
	void startup() override;
	void draw() override;
};

class PBRPass : public RenderPass {
public:
	MaterialPBR material;
	void startup() override;
	void draw() override;
};

class SkyboxPass : public RenderPass {
public:
	MaterialSkybox material;
	Skybox skybox;

	void startup() override;
	void draw() override;
};

#endif // SILENCE_RENDER_PASS_H