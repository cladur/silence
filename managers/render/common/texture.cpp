#include "texture.h"

#include "ktx.h"
#include <GLFW/glfw3.h>
#include <common/TracyColor.hpp>

std::map<std::string, ktxTexture2 *> Texture::ktx_textures;

void Texture::load_from_asset(const std::string &path, bool pregenerated_mipmaps, bool linear, bool repeat) {
	ZoneNamedNC(Zone1, "Texture::load_from_asset", tracy::Color::AntiqueWhite, true);
	ktxTexture2 *ktx_texture;
	KTX_error_code result;
	ktx_uint8_t *image;

	bool is_loaded = false;

	if (Texture::ktx_textures.contains(path)) {
		is_loaded = true;
		ktx_texture = Texture::ktx_textures[path];
		//SPDLOG_ERROR("Texture already loaded", path);
	} else {
		result = ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx_texture);
		static int not_loaded;
		not_loaded++;
		if (result != KTX_SUCCESS) {
			SPDLOG_ERROR("Couldn't load texture {}", path);
		}
		//SPDLOG_ERROR("not loaded: {}", not_loaded);
		//SPDLOG_ERROR("texture {}", path);
	}

	// TODO: Support different formats (vkFormat -> GL format)
	ktx_texture_transcode_fmt_e tf = KTX_TTF_RGBA32;
	GLenum format = GL_RGBA;

	bool is_compressed = false;
	bool bc7_supported = false;
	bool s3tc_supported = false;

	int number_of_extensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &number_of_extensions);
	for (int i = 0; i < number_of_extensions; i++) {
		const char *ccc = (const char *)glGetStringi(GL_EXTENSIONS, i);
		if ((strcmp(ccc, "GL_EXT_texture_compression_bptc") == 0) ||
				(strcmp(ccc, "GL_ARB_texture_compression_bptc") == 0)) {
			bc7_supported = true;
		} else if (strcmp(ccc, "GL_EXT_texture_compression_s3tc") == 0) {
			s3tc_supported = true;
		}
	}

	if (bc7_supported) {
		is_compressed = true;
		tf = KTX_TTF_BC7_RGBA;
		format = GL_COMPRESSED_RGBA_BPTC_UNORM;
	} else if (s3tc_supported) {
		is_compressed = true;
		tf = KTX_TTF_BC3_RGBA;
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	}

	tf = KTX_TTF_BC7_RGBA;
	format = GL_COMPRESSED_RGBA_BPTC_UNORM;

	// TODO: Check for ETC2 support?
	// tf = KTX_TTF_ETC2_RGBA;
	// format = GL_COMPRESSED_RGBA8_ETC2_EAC;

	{
		ZoneNamedNC(Zone2, "Texture::load_from_asset::ktxTexture2_TranscodeBasis", tracy::Color::AntiqueWhite1, true);
		if (!is_loaded) {
			result = ktxTexture2_TranscodeBasis(ktx_texture, tf, 0);
		}
	}
	width = ktx_texture->baseWidth;
	height = ktx_texture->baseHeight;

	GLenum target;
	{
		ZoneNamedNC(Zone3, "Texture::load_from_asset::OPENGL", tracy::Color::Black, true);
		if (ktx_texture->isCubemap) {
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_CUBE_MAP, id);
			for (unsigned int face = 0; face < 6; ++face) {
				for (unsigned int level = 0; level < ktx_texture->numLevels; ++level) {
					ktx_size_t offset = 0;
					result = ktxTexture_GetImageOffset(
							reinterpret_cast<ktxTexture *>(ktx_texture), level, 0, face, &offset);

					if (result != KTX_SUCCESS) {
						SPDLOG_ERROR("Couldn't get image offset from cubemap {}", path);
					}

					ktx_size_t size = ktxTexture_GetImageSize(reinterpret_cast<ktxTexture *>(ktx_texture), level);

					unsigned int width = ktx_texture->baseWidth * std::pow(0.5, level);
					unsigned int height = ktx_texture->baseHeight * std::pow(0.5, level);

					void *data = ktx_texture->pData + offset;

					if (is_compressed) {
						glCompressedTexImage2D(
								GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format, width, height, 0, size, data);
					} else {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format, width, height, 0, format,
								GL_UNSIGNED_BYTE, data);
					}

					// Generate mipmaps if there's only one level
					// or "generate" them to allocate the space for them if they are pregenerated
					if ((ktx_texture->numLevels == 1 || pregenerated_mipmaps) && level == 0) {
						glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
					}
				}
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);

			ktx_size_t offset = 0;
			result = ktxTexture_GetImageOffset(reinterpret_cast<ktxTexture *>(ktx_texture), 0, 0, 0, &offset);

			ktx_size_t size = ktxTexture_GetImageSize(reinterpret_cast<ktxTexture *>(ktx_texture), 0);

			void *data = ktx_texture->pData + offset;

			if (is_compressed) {
				glCompressedTexImage2D(
						GL_TEXTURE_2D, 0, format, ktx_texture->baseWidth, ktx_texture->baseHeight, 0, size, data);
			} else {
				glTexImage2D(GL_TEXTURE_2D, 0, format, ktx_texture->baseWidth, ktx_texture->baseHeight, 0, format,
						GL_UNSIGNED_BYTE, data);
			}

			if (linear) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			if (repeat) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	width = ktx_texture->baseWidth;
	height = ktx_texture->baseHeight;
	channels = 4;
}
