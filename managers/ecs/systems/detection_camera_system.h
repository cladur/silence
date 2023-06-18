#ifndef SILENCE_DETECTION_CAMERA_SYSTEM_H
#define SILENCE_DETECTION_CAMERA_SYSTEM_H

#include "base_system.h"

struct DetectionCamera;

class DetectionCameraSystem : public BaseSystem {
private:
	bool first_frame = true;

	void update_light_color(World &world, DetectionCamera &detection_camera);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_DETECTION_CAMERA_SYSTEM_H