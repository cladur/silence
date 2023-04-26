#ifndef SILENCE_OPENGL_CONTEXT_H
#define SILENCE_OPENGL_CONTEXT_H

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

#include <glad/glad.h>

#include "opengl/shader.h"

class OpenGLContext {
private:
	GLFWwindow *window;
	Shader irradiance_shader;

public:
	void startup();
	void generate_irradiance_map(const std::string &path, const std::string &export_dir);
};

#endif //SILENCE_OPENGL_CONTEXT_H
