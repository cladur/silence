#include "game.h"
#include <spdlog/spdlog.h>

#ifdef WIN32
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include <random>

int main() {
	auto start = std::chrono::high_resolution_clock::now();

	Game game;
	game.startup();

	game.create_scene("Main");
	game.scenes[0]->register_game_systems();
	game.scenes[0]->load_from_file("resources/scenes/level_test_7.scn");
	game.set_active_scene("Main");

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	SPDLOG_CRITICAL(duration);

	game.run();
	return 0;
}