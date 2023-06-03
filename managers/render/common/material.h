#ifndef SILENCE_MATERIAL_H
#define SILENCE_MATERIAL_H

#include "components/transform_component.h"
#include "render/transparent_elements/transparent_object.h"
#include "shader.h"

struct ModelInstance;
struct Mesh;

struct SkinnedModelInstance;
struct SkinnedMesh;

struct Light;

struct RenderScene;

enum class MaterialType { Default, PBR };

class Material {
public:
	Shader shader;

	virtual void startup() = 0;
	// TODO: Shutdown
	virtual void bind_resources(RenderScene &scene) = 0;
	virtual void bind_instance_resources(ModelInstance &instance, Transform &transform) = 0;
};

class MaterialPBR : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(Mesh &mesh);
};

class MaterialLight : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_light_resources(Light &light, Transform &transform);
};

class MaterialAO : public Material {
private:
	unsigned int noise_texture_id;
	std::vector<glm::vec3> ssao_kernel;

public:
	float radius = 0.4f;
	float bias = 0.04f;
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

class MaterialAOBlur : public Material {
private:
	std::vector<glm::vec2> offsets;
	std::vector<float> gauss_kernel;

public:
	int should_blur = 1;
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

class MaterialSkybox : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

class MaterialTransparent : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_object_resources(RenderScene &scene, TransparentObject &object);
};

class MaterialGBuffer : public Material {
	Shader skinned_shader;

public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_skinned_resources(RenderScene &scene);
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(Mesh &mesh, bool highlighted = false);
	void bind_instance_resources(SkinnedModelInstance &instance, Transform &transform);
	void bind_mesh_resources(SkinnedMesh &mesh);
};

class MaterialCombination : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

class MaterialBloom : public Material {
public:
	Shader downsample;
	Shader bloom;
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

class MaterialShadow : public Material {
	Shader skinned_shader;

public:
	glm::vec3 current_light_position;
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_skinned_resources(RenderScene &scene);
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_instance_resources(SkinnedModelInstance &instance, Transform &transform);
	void bind_light_resources(Light &light, Transform &transform);
};

class MaterialMousePick : public Material {
public:
	Shader shader;
	Shader skinned_shader;
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;

	void bind_instance_resources(ModelInstance &instance, Transform &transform, Entity entity);
	void bind_instance_resources(SkinnedModelInstance &instance, Transform &transform, Entity entity);
};

class MaterialParticle : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

#endif // SILENCE_MATERIAL_H