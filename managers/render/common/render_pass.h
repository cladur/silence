#ifndef SILENCE_RENDER_PASS_H
#define SILENCE_RENDER_PASS_H

#include "components/light_component.h"
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
	bool highlighted;
	glm::vec3 highlight_color;
	Entity entity;
};

struct SkinnedDrawCommand {
	SkinnedModelInstance *model_instance;
	Transform *transform;
	Entity entity;
};

struct LightDrawCommand {
	Light *light;
	Transform *transform;
};

class RenderPass {
public:
	virtual void startup() = 0;
	virtual void draw(RenderScene &scene) = 0;
};

class PBRPass : public RenderPass {
public:
	MaterialPBR material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class LightPass : public RenderPass {
public:
	MaterialLight material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class AOPass : public RenderPass {
public:
	MaterialAO material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class AOBlurPass : public RenderPass {
public:
	MaterialAOBlur material;
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
	unsigned int vao, vbo, ebo;
	std::vector<TransparentObject> screen_space_objects;
	std::vector<TransparentObject> world_space_objects;

public:
	MaterialTransparent material;
	void startup() override;
	void draw(RenderScene &scene) override;
	void sort_objects(RenderScene &scene);
	void draw_worldspace(RenderScene &scene);
	void draw_screenspace(RenderScene &scene);
};

class CombinationPass : public RenderPass {
public:
	MaterialCombination material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class BloomPass : public RenderPass {
	Handle<Texture> dirt_texture;
	float dirt_offsets[2];

public:
	MaterialBloom material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class ShadowPass : public RenderPass {
public:
	MaterialShadow material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class MousePickPass : public RenderPass {
public:
	MaterialMousePick material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

class ParticlePass : public RenderPass {
	unsigned int vao, vbo, ebo, ssbo;
public:
	MaterialParticle material;
	void startup() override;
	void draw(RenderScene &scene) override;
};

#endif // SILENCE_RENDER_PASS_H