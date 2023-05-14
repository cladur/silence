#ifndef SILENCE_RENDER_PASS_H
#define SILENCE_RENDER_PASS_H

#include "components/transform_component.h"
#include "managers/render/ecs/model_instance.h"
#include "managers/render/ecs/skinned_model_instance.h"
#include "material.h"
#include "model.h"
#include "resource/resource_manager.h"
#include "skybox.h"

struct RenderScene;

struct DrawCommand {
	ModelInstance *model_instance;
	Transform *transform;
};

struct SkinnedDrawCommand {
	SkinnedModelInstance *model_instance;
	Transform *transform;
};

class SkinnedPass {
public:
	std::vector<SkinnedDrawCommand> draw_commands;

	virtual void startup() = 0;
	virtual void draw(RenderScene &scene) = 0;
};

class SkinnedPassUnlit : public SkinnedPass {
public:
	std::vector<SkinnedDrawCommand> draw_commands;

	MaterialSkinnedUnlit material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class RenderPass {
public:
	std::vector<DrawCommand> draw_commands;

	virtual void startup() = 0;
	virtual void draw(RenderScene &scene) = 0;
};

class PBRPass : public RenderPass {
public:
	MaterialPBR material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class SkyboxPass : public RenderPass {
public:
	MaterialSkybox material;
	Skybox skybox;

	void startup() override;
	void draw(RenderScene &scene) override;
};

class GBufferPass : public RenderPass {
public:
	MaterialGBuffer material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class TransparentPass : public RenderPass {
private:
	//  render data
	// todo: create issue if this is appropriate
	// i have decided this is better than having each transparent object have its own vao, vbo, ebo
	// setting them up each time a sprite/text is created seems like worse idea than to just create them once
	// and then just SubBufferData each time.
	unsigned int vao, vbo, ebo;

public:
	MaterialTransparent material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

#endif // SILENCE_RENDER_PASS_H