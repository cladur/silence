#include "editor.h"

int main() {
	Editor &editor = *Editor::get();

	editor.startup();
	editor.run();

	return 0;
}
