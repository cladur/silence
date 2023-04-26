#include "texture.h"

#include "ktx.h"

void Texture::load_from_asset(const std::string &path) {
	ktxTexture2 *ktx_texture;
	KTX_error_code result;
	ktx_uint8_t *image;

	result = ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx_texture);

	if (result != KTX_SUCCESS) {
		SPDLOG_ERROR("Couldn't load texture {}", path);
	}

	// TODO: Support different formats (vkFormat -> GL format)
	ktx_texture_transcode_fmt_e tf = KTX_TTF_RGBA32;

	result = ktxTexture2_TranscodeBasis(ktx_texture, tf, 0);

	GLenum target;
	if (ktx_texture->isCubemap) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		for (unsigned int face = 0; face < 6; ++face) {
			ktx_size_t offset = 0;
			result = ktxTexture_GetImageOffset(reinterpret_cast<ktxTexture *>(ktx_texture), 0, 0, face, &offset);

			if (result != KTX_SUCCESS) {
				SPDLOG_ERROR("Couldn't get image offset from cubemap {}", path);
			}

			void *data = ktx_texture->pData + offset;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, ktx_texture->baseWidth,
					ktx_texture->baseHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ktx_texture->baseWidth, ktx_texture->baseHeight, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, ktx_texture->pData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	width = ktx_texture->baseWidth;
	height = ktx_texture->baseHeight;
	channels = 4;
}
