#ifndef SILENCE_BILLBOARD_SYSTEM_H
#define SILENCE_BILLBOARD_SYSTEM_H

#include "base_system.h"

class BillboardSystem : public BaseSystem {
private:
	std::string ui_scene_name = "editor_placed_billboards";
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_BILLBOARD_SYSTEM_H
