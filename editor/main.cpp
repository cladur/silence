#include "ecs/ecs_manager.h"
#include "editor.h"

int main() {
	Editor &editor = *Editor::get();

	ECSManager &ecs_manager = ECSManager::get();

	editor.startup();
	ecs_manager.print_components();
	editor.run();

	return 0;
}
