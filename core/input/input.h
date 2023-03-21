#ifndef SILENCE_INPUT_H
#define SILENCE_INPUT_H

class Input {
	static Input *singleton;

public:
	enum MouseMode {
		MOUSE_MODE_VISIBLE,
		MOUSE_MOVE_HIDDEN,
		MOUSE_MODE_CAPTURED,
	};

	enum CursorShape {
		CURSOR_ARROW,
		// TODO: Handle all other cursor types
	};

	enum {
		JOYPADS_MAX = 16,
	};
};

#endif //SILENCE_INPUT_H
