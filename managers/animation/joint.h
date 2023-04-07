#ifndef SILENCE_JOINT_H
#define SILENCE_JOINT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Joint {
public:
	int32_t id;

	/** Compressed position */
	uint16_t position[3];
	/** Compressed rotation */
	uint16_t rotation[3];

	static inline glm::vec3 get_glm_vec3(const std::vector<double> &vec) {
		if (vec.size() < 3) {
			SPDLOG_WARN("Failed to get glm::vec3, array has insufficient elements");
			return {};
		}
		return { static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2]) };
	}

	static inline glm::quat get_glm_quat(const std::vector<double> &quat) {
		if (quat.size() < 4) {
			SPDLOG_WARN("Failed to get glm::quat, array has insufficient elements");
			return {};
		}
		return glm::quat(static_cast<float>(quat[0]), static_cast<float>(quat[1]), static_cast<float>(quat[2]),
				static_cast<float>(quat[3]));
	}

	static void quat_to_uint16(const glm::quat &q, uint16_t *result) {
		glm::vec3 vec = quat_to_vec3(q);
		vec3_to_uint16(vec, result);
	}

	static glm::quat uint16_to_quat(const uint16_t *b) {
		glm::vec3 vec = uint16_to_vec3(b);
		return vec3_to_quat(vec);
	}

	static void vec4_to_uint16(const glm::vec4 &v, uint16_t *result) {
		glm::vec3 vec = glm::vec3(v.x / PosRange, v.y / PosRange, v.z / PosRange);
		vec3_to_uint16(vec, result);
	}

	static glm::vec4 uint16_to_vec4(const uint16_t *b) {
		glm::vec3 vec = uint16_to_vec3(b);

		glm::vec4 result;
		result.x = vec.x * PosRange;
		result.y = vec.y * PosRange;
		result.z = vec.z * PosRange;
		result.w = 0.0f;

		return result;
	}

	static glm::mat4 get_glm_matrix(const float *matrix, int32_t index) {
		return glm::mat4(matrix[index], matrix[index + 1], matrix[index + 2], matrix[index + 3], matrix[index + 4],
				matrix[index + 5], matrix[index + 6], matrix[index + 7], matrix[index + 8], matrix[index + 9],
				matrix[index + 10], matrix[index + 11], matrix[index + 12], matrix[index + 13], matrix[index + 14],
				matrix[index + 15]);
	}

	static glm::quat get_glm_rot(const float *matrix, int32_t index) {
		glm::quat result;
		result.w = sqrt(1.0f + matrix[index + 0] + matrix[index + 5] + matrix[index + 10]) / 2.0f;
		result.x = (matrix[index + 9] - matrix[index + 6]) / (4.0f * result.w);
		result.y = (matrix[index + 2] - matrix[index + 8]) / (4.0f * result.w);
		result.z = (matrix[index + 4] - matrix[index + 1]) / (4.0f * result.w);
		return result;
	}

	static glm::vec3 get_glm_pos(const float *matrix, int32_t index) {
		return glm::vec3(matrix[index + 12], matrix[index + 13], matrix[index + 14]);
	}

	static glm::quat get_glm_rot(const double *matrix, int32_t index) {
		glm::quat result;
		result.w = static_cast<float>(sqrt(1.0 + matrix[index + 0] + matrix[index + 5] + matrix[index + 10]) / 2.0);
		result.x = static_cast<float>((matrix[index + 9] - matrix[index + 6]) / (4.0 * result.w));
		result.y = static_cast<float>((matrix[index + 2] - matrix[index + 8]) / (4.0 * result.w));
		result.z = static_cast<float>((matrix[index + 4] - matrix[index + 1]) / (4.0 * result.w));
		return result;
	}

	static glm::vec3 get_glm_pos(const double *matrix, int32_t index) {
		return glm::vec3(matrix[index + 12], matrix[index + 13], matrix[index + 14]);
	}

private:
	// 4(sqrt(2)-1)
	static inline const float Km = 4.0f * 0.4142135679721832275390625f;
	// sqrt(2)+1 = 1/(sqrt(2)-1)
	static inline const float Khf = 2.414213657379150390625f;
	// 3-2sqrt(2)
	static inline const float Khi = 0.17157287895679473876953125f;

	static inline const float PosRange = 20.0f;

	// compression
	static glm::vec3 quat_to_vec3(const glm::quat &q) {
		glm::vec3 v;

		float s = Khf / (1.0f + q.w + sqrt(2.0f + 2.0f * q.w));

		v.x = q.x * s;
		v.y = q.y * s;
		v.z = q.z * s;

		return v;
	}

	// decompression
	static glm::quat vec3_to_quat(const glm::vec3 &v) {
		float d = Khi * glm::dot(v, v);
		float a = (1.0f + d);
		float b = (1.0f - d) * Km;
		float c = 1.0f / (a * a);

		float bc = b * c;

		glm::quat q;
		q.x = v.x * bc;
		q.y = v.y * bc;
		q.z = v.z * bc;
		q.w = (1.0f + d * (d - 6.0f)) * c;

		return q;
	}

	static inline float decompress_float_minus_one_plus_one(uint16_t Value) {
		return (float(Value) / 65535.0f) * 2.0f - 1.0f;
	}

	static inline uint16_t compress_float_minus_one_plus_one(float Value) {
		return uint16_t(((Value + 1.0f) / 2.0f) * 65535.0f);
	}

	static void vec3_to_uint16(const glm::vec3 &v, uint16_t *result) {
		result[0] = compress_float_minus_one_plus_one(v.x);
		result[1] = compress_float_minus_one_plus_one(v.y);
		result[2] = compress_float_minus_one_plus_one(v.z);
	}

	static glm::vec3 uint16_to_vec3(const uint16_t *b) {
		glm::vec3 result;

		result.x = decompress_float_minus_one_plus_one(b[0]);
		result.y = decompress_float_minus_one_plus_one(b[1]);
		result.z = decompress_float_minus_one_plus_one(b[2]);

		return result;
	}
};

#endif //SILENCE_JOINT_H
