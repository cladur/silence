#ifndef SILENCE_MATERIAL_H
#define SILENCE_MATERIAL_H

#include "components/transform_component.h"
#include "shader.h"

struct ModelInstance;
struct Mesh;
struct RenderScene;

enum class MaterialType { Default, Unlit, PBR };

class Material {
public:
	Shader shader;

	virtual void startup() = 0;
	// TODO: Shutdown
	virtual void bind_resources(RenderScene &scene) = 0;
	virtual void bind_instance_resources(ModelInstance &instance, Transform &transform) = 0;
};

class MaterialUnlit : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(Mesh &mesh);
};

class MaterialPBR : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
	void bind_mesh_resources(Mesh &mesh);
};

class MaterialSkybox : public Material {
public:
	void startup() override;
	void bind_resources(RenderScene &scene) override;
	void bind_instance_resources(ModelInstance &instance, Transform &transform) override;
};

#endif // SILENCE_MATERIAL_H