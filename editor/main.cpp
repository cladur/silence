#include "components/rigidbody_component.h"
#include "ecs/ecs_manager.h"
#include "editor.h"

int main() {
	Editor &editor = *Editor::get();

	ECSManager &ecs_manager = ECSManager::get();

	editor.startup();
	ecs_manager.print_components();
	SPDLOG_WARN(ecs_manager.has_component(1, 2));
	ecs_manager.add_component(1, 2);
	SPDLOG_WARN(ecs_manager.has_component(1, 2));
	editor.run();

	return 0;
}
