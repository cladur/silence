#include "game.h"

int main() {
	Game game;
	game.startup();

	game.create_scene("Main");
	game.scenes[0]->load_from_file("resources/scenes/Skrzyneczka.scn");
	game.set_active_scene("Main");
	game.menu_test.startup(game.scenes[0]->get_render_scene());

	game.run();
	return 0;
}