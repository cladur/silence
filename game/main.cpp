#include "display_manager.h"
#include "render_manager.h"

#include "magic_enum.hpp"
#include "spdlog/spdlog.h"

RenderManager render_manager;
DisplayManager display_manager;

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	auto dm_ret = display_manager.startup();
	if (dm_ret == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}", magic_enum::enum_integer(dm_ret),
				magic_enum::enum_name(dm_ret));
		return -1;
	}

	auto rm_ret = render_manager.startup(display_manager);
	if (rm_ret == RenderManager::Status::Ok) {
		SPDLOG_INFO("Initialized render manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the render manager. Status: ({}) {}", magic_enum::enum_integer(rm_ret),
				magic_enum::enum_name(rm_ret));
		return -1;
	}

	// Run the game.
	bool should_run = true;
	while (should_run) {
		// GAME LOGIC

		display_manager.poll_events();

		if (display_manager.window_should_close()) {
			should_run = false;
		}
	}


	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	render_manager.shutdown();
	display_manager.shutdown();

	return 0;
}
