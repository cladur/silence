#include "asset_loader.h"

bool assets::save_binary_file(const char *path, const assets::AssetFile &file) {
	std::ofstream out_file;
	out_file.open(path, std::ios::binary | std::ios::out);

	out_file.write(file.type, 4);
	uint32_t version = file.version;
	//version
	out_file.write((const char *)&version, sizeof(uint32_t));

	//json length
	uint32_t length = file.json.size();
	out_file.write((const char *)&length, sizeof(uint32_t));

	//blob length
	uint32_t bloblength = file.binary_blob.size();
	out_file.write((const char *)&bloblength, sizeof(uint32_t));

	//json stream
	out_file.write(file.json.data(), length);
	//blob data
	out_file.write(file.binary_blob.data(), (long)file.binary_blob.size());

	out_file.close();

	return true;
}

bool assets::load_binary_file(const char *path, assets::AssetFile &output_file) {
	std::ifstream in_file;
	in_file.open(path, std::ios::binary);

	if (!in_file.is_open()) {
		return false;
	}

	//move file cursor to beginning
	in_file.seekg(0);

	// TODO: Error checking on type, version of assets and etc.

	in_file.read(output_file.type, 4);
	in_file.read((char *)&output_file.version, sizeof(uint32_t));

	uint32_t jsonlen = 0;
	in_file.read((char *)&jsonlen, sizeof(uint32_t));

	uint32_t bloblen = 0;
	in_file.read((char *)&bloblen, sizeof(uint32_t));

	output_file.json.resize(jsonlen);
	in_file.read(output_file.json.data(), jsonlen);

	output_file.binary_blob.resize(bloblen);
	in_file.read(output_file.binary_blob.data(), bloblen);

	return true;
}

assets::CompressionMode assets::parse_compression(const char *f) {
	if (strcmp(f, "LZ4") == 0) {
		return assets::CompressionMode::LZ4;
	} else {
		return assets::CompressionMode::None;
	}
}
