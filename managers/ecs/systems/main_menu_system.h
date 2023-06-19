#ifndef SILENCE_MAIN_MENU_SYSTEM_H
#define SILENCE_MAIN_MENU_SYSTEM_H

#include "base_system.h"

struct MainMenu;

class MainMenuSystem : public BaseSystem {
	std::string ui_name;
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
	void init_ui(MainMenu &menu);
};

#endif //SILENCE_MAIN_MENU_SYSTEM_H
