#ifndef SILENCE_DETECTION_CAMERA_SYSTEM_H
#define SILENCE_DETECTION_CAMERA_SYSTEM_H

#include "base_system.h"

class DetectionCameraSystem : public BaseSystem {
private:
	bool first_frame = true;

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_DETECTION_CAMERA_SYSTEM_H