#include "game.h"

int main() {
	Game game;
	game.startup();

	game.create_scene("Main");
	game.scenes[0]->load_from_file("resources/scenes/Skrzyneczka.scn");

	game.run();
	return 0;
}