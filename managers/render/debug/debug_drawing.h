#ifndef SILENCE_DEBUG_DRAWING_H
#define SILENCE_DEBUG_DRAWING_H

#include <render/render_manager.h>

class DebugDraw {
private:

public:
    DebugDraw();

    ~DebugDraw();

	// Draw a line from "from" to "to". Beginning of the line is red turns to blue at the end.
    static void draw_line(const glm::vec3 &from, const glm::vec3 &to);

	// Draw a box with center at "center" and scale "scale".
	static void draw_box(const glm::vec3 &center, const glm::vec3 &scale);

	// Draw a sphere with center at "center" and radius "radius".
	static void draw_sphere(const glm::vec3 &center, float radius);
};

#endif //SILENCE_DEBUG_DRAWING_H
