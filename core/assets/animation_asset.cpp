#include "animation_asset.h"

#include "lz4.h"
#include <nlohmann/json.hpp>

assets::AssetFile assets::pack_animation(assets::AnimationInfo *info, char *node_data) {
	AssetFile file;
	file.type[0] = 'A';
	file.type[1] = 'N';
	file.type[2] = 'I';
	file.type[3] = 'M';
	file.version = 1;

	nlohmann::json metadata;
	metadata["full_size"] = info->full_size;

	std::vector<char> merged_buffer;
	merged_buffer.resize(info->full_size);

	metadata["original_file"] = info->original_file;
	for (auto &node : info->sizes) {
		metadata["channel_" + std::to_string(node.first)]["translations_buffer_size"] = node.second.translations;
		metadata["channel_" + std::to_string(node.first)]["translations_times_buffer_size"] =
				node.second.translation_times;
		metadata["channel_" + std::to_string(node.first)]["rotations_buffer_size"] = node.second.rotations;
		metadata["channel_" + std::to_string(node.first)]["rotations_times_buffer_size"] = node.second.rotation_times;
		metadata["channel_" + std::to_string(node.first)]["name"] = info->node_names[node.first];

		size_t node_size = node.second.translations + node.second.translation_times + node.second.rotations +
				node.second.rotation_times;

		//copy vertex buffer
		memcpy(merged_buffer.data(), node_data, node_size);
	}

	//compress buffer and copy it into the file struct
	size_t compress_staging = LZ4_compressBound(static_cast<int>(info->full_size));

	file.binary_blob.resize(compress_staging);

	int compressed_size = LZ4_compress_default(merged_buffer.data(), file.binary_blob.data(),
			static_cast<int>(merged_buffer.size()), static_cast<int>(compress_staging));
	file.binary_blob.resize(compressed_size);

	metadata["compression"] = "LZ4";

	file.json = metadata.dump();

	return file;
}
