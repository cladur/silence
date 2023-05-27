#ifndef SILENCE_FRAMEBUFFER_H
#define SILENCE_FRAMEBUFFER_H

#include <stdint.h>
class Framebuffer {
public:
	uint32_t framebuffer_id;
	uint32_t texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

class GBuffer {
public:
	uint32_t framebuffer_id;
	uint32_t position_texture_id;
	uint32_t normal_texture_id;
	uint32_t albedo_texture_id;
	uint32_t ao_rough_metal_texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

class SSAOBuffer {
public:
	uint32_t framebuffer_id;
	uint32_t ssao_texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

class PBRBuffer {
public:
	uint32_t framebuffer_id;
	uint32_t diffuse_texture_id;
	uint32_t specular_texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

class CombinationBuffer {
public:
	uint32_t framebuffer_id;
	uint32_t texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

struct BloomMip {
	glm::vec2 size;
	glm::ivec2 int_size;
	unsigned int texture_id;
};

class BloomBuffer {
public:
	uint32_t framebuffer_id;
	uint32_t bloom_texture_id;
	uint32_t rbo_id;

	std::vector<BloomMip> mips;

	void startup(uint32_t width, uint32_t height, int mip_count);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

class SkyboxBuffer {
public:
	uint32_t framebuffer_id;
	uint32_t texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

class MousePickFramebuffer {
public:
	uint32_t framebuffer_id;
	uint32_t texture_id;
	uint32_t rbo_id;

	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
};

#endif // SILENCE_FRAMEBUFFER_H