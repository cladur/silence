#ifndef SILENCE_MATERIAL_H
#define SILENCE_MATERIAL_H

#include "components/transform_component.h"
#include "render/transparent_elements/transparent_object.h"
#include "shader.h"

struct ModelInstance;
struct Mesh;

struct SkinnedModelInstance;
struct SkinnedMesh;

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

class MaterialSkinned {
public:
	Shader shader;

	virtual void startup() = 0;
	// TODO: Shutdown
	virtual void bind_resources(RenderScene &scene) = 0;
	virtual void bind_instance_resources(SkinnedModelInstance &instance, Transform &transform) = 0;
};

class MaterialSkinnedUnlit : public MaterialSkinned {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(SkinnedModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(SkinnedMesh &mesh);
};

class MaterialPBR : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(Mesh &mesh);
};

class MaterialAO : public Material {
private:
	unsigned int noise_texture_id;
	std::vector<glm::vec3> ssao_kernel;
public:
	float radius = 0.5f;
	float bias = 0.025f;
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
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(Mesh &mesh);
};

#endif // SILENCE_MATERIAL_H