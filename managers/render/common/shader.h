#ifndef SILENCE_SHADER_H
#define SILENCE_SHADER_H

#include <glad/glad.h>

class Shader {
public:
	// the program ID
	unsigned int id;

	// constructor reads and builds the shader
	void load_from_files(
			const std::string &vertex_path, const std::string &fragment_path, const std::string &geometry_path = "");
	// use/activate the shader
	void use();
	// utility uniform functions
	void set_bool(const std::string &name, bool value) const;
	void set_int(const std::string &name, int value) const;
	void set_uint(const std::string &name, unsigned int value) const;
	void set_float(const std::string &name, float value) const;
	void set_vec2(const std::string &name, const glm::vec2 &value) const;
	void set_vec2(const std::string &name, float x, float y) const;
	void set_vec3(const std::string &name, const glm::vec3 &value) const;
	void set_vec3(const std::string &name, float x, float y, float z) const;
	void set_vec4(const std::string &name, const glm::vec4 &value) const;
	void set_vec4(const std::string &name, float x, float y, float z, float w) const;
	void set_mat2(const std::string &name, const glm::mat2 &mat) const;
	void set_mat3(const std::string &name, const glm::mat3 &mat) const;
	void set_mat4(const std::string &name, const glm::mat4 &mat) const;

private:
	void check_compile_errors(GLuint shader, std::string type);
};

#endif // SILENCE_SHADER_H