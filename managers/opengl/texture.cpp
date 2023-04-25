#include "texture.h"

#include "ktx.h"

void Texture::load_from_asset(const char *path) {
	ktxTexture2 *ktx_texture;
	KTX_error_code result;
	ktx_size_t offset;
	ktx_uint8_t *image;
	ktx_uint32_t level, layer, face_slice;
	GLenum target, glerror;

	result = ktxTexture2_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_NO_FLAGS, &ktx_texture);

	if (result != KTX_SUCCESS) {
		SPDLOG_ERROR("Couldn't load texture {}", path);
	}

	// glGenTextures(1, &id); // Optional. GLUpload can generate a texture.
	// result = ktxTexture_GLUpload((ktxTexture *)ktx_texture, &id, &target, &glerror);

	// if (result != KTX_SUCCESS) {
	// 	SPDLOG_ERROR("Couldn't upload texture {}", path);
	// }

	// TODO: Support different formats (vkFormat -> GL format)
	ktx_texture_transcode_fmt_e tf = KTX_TTF_RGBA32;

	result = ktxTexture2_TranscodeBasis(ktx_texture, tf, 0);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ktx_texture->baseWidth, ktx_texture->baseHeight, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, ktx_texture->pData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// ktxTexture_Destroy(id);
}
