#include "editor.h"

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main() {
	Editor &editor = *Editor::get();

	editor.startup();
	editor.run();

	return 0;
}
