#include "game.h"

#ifdef WIN32
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main() {
	Game game;
	game.startup();

	game.create_scene("Main");
	game.scenes[0]->register_game_systems();
	game.scenes[0]->load_from_file("resources/scenes/level_0test.scn");
	game.set_active_scene("Main");

	game.run();
	return 0;
}