#include "game.h"

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main() {
	Game game;
	game.startup();

	game.create_scene("Main");
	game.scenes[0]->load_from_file("resources/scenes/collision_test.scn");
	game.set_active_scene("Main");
	//game.menu_test.startup(game.scenes[0]->get_render_scene());

	game.run();
	return 0;
}