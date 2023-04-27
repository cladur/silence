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
	Shader prefilter_shader;
	Shader brdf_shader;

public:
	void startup();
	void process_cubemap(const std::string &path, const std::string &export_dir);
};

#endif //SILENCE_OPENGL_CONTEXT_H
