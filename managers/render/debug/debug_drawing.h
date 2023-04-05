#ifndef SILENCE_DEBUG_DRAWING_H
#define SILENCE_DEBUG_DRAWING_H

#include <render/render_manager.h>

class DebugDraw {
private:

public:
    DebugDraw();

    ~DebugDraw();

    static void draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color);

	static void draw_box(const glm::vec3 &center, const glm::vec3 &scale);

	static void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color);
};

#endif //SILENCE_DEBUG_DRAWING_H
