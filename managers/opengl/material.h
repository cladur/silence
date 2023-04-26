#ifndef SILENCE_MATERIAL_H
#define SILENCE_MATERIAL_H

#include "shader.h"

struct ModelInstance;

enum MaterialType { MATERIAL_TYPE_UNLIT, MATERIAL_TYPE_PBR, MATERIAL_TYPE_COUNT };

class Material {
public:
	Shader shader;

	virtual void startup() = 0;
	// TODO: Shutdown
	virtual void bind_resources() = 0;
	virtual void bind_instance_resources(ModelInstance &instance) = 0;
};

class MaterialUnlit : public Material {
public:
	void startup() override;
	void bind_resources() override;
	void bind_instance_resources(ModelInstance &instance) override;
};

class MaterialPBR : public Material {
public:
	void startup() override;
	void bind_resources() override;
	void bind_instance_resources(ModelInstance &instance) override;
};

class MaterialSkybox : public Material {
public:
	void startup() override;
	void bind_resources() override;
	void bind_instance_resources(ModelInstance &instance) override;
};

#endif // SILENCE_MATERIAL_H