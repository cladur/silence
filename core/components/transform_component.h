#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#ifndef SILENCE_TRANSFORM_H
#define SILENCE_TRANSFORM_H

struct Transform {
	glm::vec3 position;
	glm::vec3 euler_rot;
	glm::vec3 scale;

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::mat4 get_local_model_matrix() {
		const glm::mat4 transform_x =
				glm::rotate(glm::mat4(1.0f), glm::radians(euler_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transform_y =
				glm::rotate(glm::mat4(1.0f), glm::radians(euler_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transform_z =
				glm::rotate(glm::mat4(1.0f), glm::radians(euler_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

		// Y * X * Z
		const glm::mat4 roation_matrix = transform_y * transform_x * transform_z;

		// translation * rotation * scale (also known as TRS matrix)
		this->modelMatrix =
				glm::translate(glm::mat4(1.0f), position) * roation_matrix * glm::scale(glm::mat4(1.0f), scale);
		return this->modelMatrix;
	}
};

#endif //SILENCE_TRANSFORM_H