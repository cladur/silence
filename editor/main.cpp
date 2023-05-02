#include "components/rigidbody_component.h"
#include "ecs/ecs_manager.h"
#include "editor.h"

int main() {
	Editor &editor = *Editor::get();

	editor.startup();
	editor.run();

	return 0;
}
