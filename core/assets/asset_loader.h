#ifndef SILENCE_ASSET_LOADER_H
#define SILENCE_ASSET_LOADER_H

namespace assets {
struct AssetFile {
	char type[4];
	int version;
	std::string json;
	std::vector<char> binary_blob;
};

enum class CompressionMode : uint32_t { None, LZ4 };

bool save_binary_file(const char *path, const AssetFile &file);
bool load_binary_file(const char *path, AssetFile &output_file);

assets::CompressionMode parse_compression(const char *f);

} //namespace assets

#endif //SILENCE_ASSET_LOADER_H
