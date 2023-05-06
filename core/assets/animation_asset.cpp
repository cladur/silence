#include "animation_asset.h"

#include "lz4.h"
#include <nlohmann/json.hpp>

assets::AnimationInfo assets::read_animation_info(assets::AssetFile *file) {
	AnimationInfo info;

	nlohmann::json metadata = nlohmann::json::parse(file->json);

	info.full_size = metadata["full_size"];
	info.original_file = metadata["original_file"];
	info.duration_seconds = metadata["duration_seconds"];
	for (auto &[key, value] : metadata["node_names"].items()) {
		info.node_names.push_back(value[1]);
	}

	std::vector<nlohmann::json> channels = metadata["channels"];
	info.sizes.resize(channels.size());
	for (int32_t i = 0; i < channels.size(); ++i) {
		info.sizes[i].translations = channels[i]["translations"];
		info.sizes[i].rotations = channels[i]["rotations"];
		info.sizes[i].translation_times = channels[i]["translation_times"];
		info.sizes[i].rotation_times = channels[i]["rotation_times"];
	}

	return info;
}

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
	nlohmann::json channels;
	size_t actual_offset = 0;
	for (auto &node : info->sizes) {
		nlohmann::json channel;
		channel["translations"] = node.translations;
		channel["rotations"] = node.rotations;
		channel["translation_times"] = node.translation_times;
		channel["rotation_times"] = node.rotation_times;

		channels.push_back(channel);
		size_t node_size = node.translations + node.translation_times + node.rotations + node.rotation_times;

		//copy node data
		memcpy(merged_buffer.data() + actual_offset, node_data, node_size);
		actual_offset += node_size;
	}

	metadata["channels"] = channels;
	metadata["duration_seconds"] = info->duration_seconds;
	metadata["node_names"] = info->node_names;

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

void assets::unpack_animation(
		AnimationInfo *info, const char *source_buffer, std::vector<assets::NodeAnimation> &nodes) {
	//decompressing into temporal vector. TODO: streaming decompress directly on the buffers
	std::vector<char> decompressed_buffer;
	decompressed_buffer.resize(info->full_size);

	LZ4_decompress_safe(source_buffer, decompressed_buffer.data(), static_cast<int>(info->full_size),
			static_cast<int>(decompressed_buffer.size()));

	size_t actual_offset = 0;
	for (int32_t i = 0; i < info->sizes.size(); ++i) {
		nodes[i].rotations.resize(info->sizes[i].rotations);
		memcpy(nodes[i].rotations.data(), decompressed_buffer.data() + actual_offset, nodes[i].rotations.size());
		actual_offset += nodes[i].rotations.size();

		nodes[i].translations.resize(info->sizes[i].translations);
		memcpy(nodes[i].translations.data(), decompressed_buffer.data() + actual_offset, nodes[i].translations.size());
		actual_offset += nodes[i].translations.size();

		nodes[i].translation_times.resize(info->sizes[i].translation_times);
		memcpy(nodes[i].translation_times.data(), decompressed_buffer.data() + actual_offset,
				nodes[i].translation_times.size());
		actual_offset += nodes[i].translation_times.size();

		nodes[i].rotation_times.resize(info->sizes[i].rotation_times);
		memcpy(nodes[i].rotation_times.data(), decompressed_buffer.data() + actual_offset,
				nodes[i].rotation_times.size());
		actual_offset += nodes[i].rotation_times.size();
	}
}
