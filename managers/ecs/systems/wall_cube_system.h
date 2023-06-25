#ifndef SILENCE_WALL_CUBE_SYSTEM_H
#define SILENCE_WALL_CUBE_SYSTEM_H

#include "base_system.h"

struct ModelInstance;

class WallCubeSystem : public BaseSystem {
private:
	void update_face_uv(ModelInstance &face_model_instance, int i, glm::vec3 global_scale);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_WALL_CUBE_SYSTEM_H
