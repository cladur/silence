#ifndef SILENCE_FRAMEBUFFER_H
#define SILENCE_FRAMEBUFFER_H

class Framebuffer {
private:
	uint32_t framebuffer_id;
	uint32_t texture_id;
	uint32_t rbo_id;

public:
	void startup(uint32_t width, uint32_t height);
	void bind();
	void resize(uint32_t width, uint32_t height);
	[[nodiscard]] uint32_t get_texture_id() const;
};

#endif // SILENCE_FRAMEBUFFER_H