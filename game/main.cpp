#include "game.h"

int main() {
	Game game;
	game.startup();

	game.create_scene("Main");
	game.scenes[0]->load_from_file("resources/scenes/level_0.scn");
	game.set_active_scene("Main");

	game.run();
	return 0;
}