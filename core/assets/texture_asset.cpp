#include "texture_asset.h"

#include <ktx.h>
#include <lz4.h>
#include <nlohmann/json.hpp>

assets::TextureFormat parse_texture_format(const char *f) {
	if (strcmp(f, "RGBA8") == 0) {
		return assets::TextureFormat::RGBA8;
	} else {
		return assets::TextureFormat::Unknown;
	}
}

assets::TextureInfo assets::read_texture_info(AssetFile *file) {
	TextureInfo info;

	nlohmann::json texture_metadata = nlohmann::json::parse(file->json);

	std::string format_string = texture_metadata["format"];
	info.texture_format = parse_texture_format(format_string.c_str());

	std::string compression_string = texture_metadata["compression"];
	info.compression_mode = parse_compression(compression_string.c_str());

	info.pixel_size[0] = texture_metadata["width"];
	info.pixel_size[1] = texture_metadata["height"];
	info.texture_size = texture_metadata["buffer_size"];
	info.original_file = texture_metadata["original_file"];

	return info;
}

assets::AssetFile assets::pack_texture(assets::TextureInfo *info, void *pixel_data) {
	nlohmann::json texture_metadata;
	texture_metadata["format"] = "RGBA8";
	texture_metadata["width"] = info->pixel_size[0];
	texture_metadata["height"] = info->pixel_size[1];
	texture_metadata["buffer_size"] = info->texture_size;
	texture_metadata["original_file"] = info->original_file;

	//core file header
	AssetFile file;
	file.type[0] = 'T';
	file.type[1] = 'E';
	file.type[2] = 'X';
	file.type[3] = 'I';
	file.version = 1;

	//compress buffer into blob
	int compress_staging = LZ4_compressBound((int)info->texture_size);

	file.binary_blob.resize(compress_staging);

	int compressed_size = LZ4_compress_default(
			(const char *)pixel_data, file.binary_blob.data(), (int)info->texture_size, compress_staging);

	file.binary_blob.resize(compressed_size);

	texture_metadata["compression"] = "LZ4";

	std::string stringified = texture_metadata.dump();
	file.json = stringified;

	return file;
}

void assets::unpack_texture(TextureInfo *info, const char *source_buffer, size_t source_size, char *destination) {
	if (info->compression_mode == CompressionMode::LZ4) {
		LZ4_decompress_safe(source_buffer, destination, (int)source_size, (int)info->texture_size);
	} else {
		memcpy(destination, source_buffer, source_size);
	}
}