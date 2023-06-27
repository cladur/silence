#include "texture.h"

#include "ktx.h"
#include <GLFW/glfw3.h>

std::map<std::string, ktxTexture2 *> Texture::ktx_textures;

GLenum Texture::get_supported_compressed_format() {
	static bool first_time = true;
	static bool bc7_supported = false;
	static bool s3tc_supported = false;

	if (first_time) {
		ZoneNamedNC(Zone2, "Texture::load_from_asset::check_supported_extensions", tracy::Color::AntiqueWhite1, true);
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
		first_time = false;
	}

	if (bc7_supported) {
		return GL_COMPRESSED_RGBA_BPTC_UNORM;
	} else if (s3tc_supported) {
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	} else {
		return GL_RGBA;
	}
}

void Texture::load_from_asset(const std::string &path, bool pregenerated_mipmaps, bool repeat) {
	ZoneNamedNC(Zone1, "Texture::load_from_asset", tracy::Color::AntiqueWhite, true);
	ktxTexture2 *ktx_texture;
	KTX_error_code result;
	ktx_uint8_t *image;
	name = path;

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
	bool is_compressed = false;

	GLenum format = get_supported_compressed_format();

	if (format == GL_COMPRESSED_RGBA_BPTC_UNORM) {
		is_compressed = true;
		tf = KTX_TTF_BC7_RGBA;
	} else if (format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
		is_compressed = true;
		tf = KTX_TTF_BC3_RGBA;
	}

	{
		ZoneNamedNC(Zone2, "Texture::load_from_asset::ktxTexture2_TranscodeBasis", tracy::Color::AntiqueWhite1, true);
		if (!is_loaded) {
			result = ktxTexture2_TranscodeBasis(ktx_texture, tf, 0);
		}
	}
	width = ktx_texture->baseWidth;
	height = ktx_texture->baseHeight;

	float aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);

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

					if (width == 0) {
						width = 1;
					}
					if (height == 0) {
						height = 1;
					}

					void *data = ktx_texture->pData + offset;

					if (is_compressed) {
						glCompressedTexImage2D(
								GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format, width, height, 0, size, data);
					} else {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format, width, height, 0, format,
								GL_UNSIGNED_BYTE, data);
					}
				}
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, ktx_texture->numLevels - 1);

			// Generate mipmaps if there's only one level
			if (ktx_texture->numLevels == 1) {
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			}
		} else {
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

			for (unsigned int level = 0; level < ktx_texture->numLevels; level++) {
				ktx_size_t offset = 0;
				result = ktxTexture_GetImageOffset(reinterpret_cast<ktxTexture *>(ktx_texture), level, 0, 0, &offset);

				ktx_size_t size = ktxTexture_GetImageSize(reinterpret_cast<ktxTexture *>(ktx_texture), level);

				void *data = ktx_texture->pData + offset;

				GLsizei width = ktx_texture->baseWidth >> level;
				GLsizei height = ktx_texture->baseHeight >> level;

				if (width == 0) {
					width = 1;
				}
				if (height == 0) {
					height = 1;
				}

				if (is_compressed) {
					glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, data);
				} else {
					glTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				}
			}

			if (repeat) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, ktx_texture->numLevels - 1);

			// Generate mipmaps if there's only one level
			if (ktx_texture->numLevels == 1) {
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}

	width = ktx_texture->baseWidth;
	height = ktx_texture->baseHeight;
	channels = 4;

	ktxTexture_Destroy(reinterpret_cast<ktxTexture *>(ktx_texture));
}
