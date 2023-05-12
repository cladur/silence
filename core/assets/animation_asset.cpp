#include "animation_asset.h"

#include "lz4.h"
#include <nlohmann/json.hpp>

assets::AnimationInfo assets::read_animation_info(assets::AssetFile *file) {
	AnimationInfo info;

	nlohmann::json metadata = nlohmann::json::parse(file->json);

	info.full_size = metadata["full_size"];
	info.original_file = metadata["original_file"];
	info.duration_seconds = metadata["duration_seconds"];
	info.node_names = metadata["node_names"].get<std::vector<std::string>>();

	std::vector<nlohmann::json> channels = metadata["channels"];
	info.sizes.resize(channels.size());
	for (int32_t i = 0; i < channels.size(); ++i) {
		info.sizes[i].translations = channels[i]["translation"];
		info.sizes[i].rotations = channels[i]["rotation"];
		info.sizes[i].translation_times = channels[i]["translation_times"];
		info.sizes[i].rotation_times = channels[i]["rotation_times"];
	}

	return info;
}

assets::AssetFile assets::pack_animation(assets::AnimationInfo *info, std::vector<assets::NodeAnimation> &nodes) {
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
	for (int32_t i = 0; i < info->sizes.size(); ++i) {
		nlohmann::json channel;
		channel["translation"] = info->sizes[i].translations;
		channel["rotation"] = info->sizes[i].rotations;
		channel["translation_times"] = info->sizes[i].translation_times;
		channel["rotation_times"] = info->sizes[i].rotation_times;

		channels.push_back(channel);

		//copy node data
		memcpy(merged_buffer.data() + actual_offset, nodes[i].translations.data(), info->sizes[i].translations);
		actual_offset += info->sizes[i].translations;

		memcpy(merged_buffer.data() + actual_offset, nodes[i].translation_times.data(),
				info->sizes[i].translation_times);
		actual_offset += info->sizes[i].translation_times;

		memcpy(merged_buffer.data() + actual_offset, nodes[i].rotations.data(), info->sizes[i].rotations);
		actual_offset += info->sizes[i].rotations;

		memcpy(merged_buffer.data() + actual_offset, nodes[i].rotation_times.data(), info->sizes[i].rotation_times);
		actual_offset += info->sizes[i].rotation_times;
		// I've got a bug that appears in some models, that last value of rotation_times was wrong after load but
		// before save it was correct
		//		float z = 0;
		//		memcpy(&z, merged_buffer.data() + (actual_offset - 4), sizeof(float));
		//		SPDLOG_INFO("{}", z);
		//		for (auto &n : nodes[i].rotation_times) {
		//			SPDLOG_INFO("{}", n);
		//		}
	}
	//	SPDLOG_INFO("{} {}", actual_offset, info->full_size);

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

	nodes.resize(info->sizes.size());
	size_t actual_offset = 0;
	for (int32_t i = 0; i < info->sizes.size(); ++i) {
		nodes[i].translations.resize(info->sizes[i].translations / sizeof(glm::vec3));
		memcpy(nodes[i].translations.data(), decompressed_buffer.data() + actual_offset, info->sizes[i].translations);
		actual_offset += info->sizes[i].translations;

		nodes[i].translation_times.resize(info->sizes[i].translation_times / sizeof(float));
		memcpy(nodes[i].translation_times.data(), decompressed_buffer.data() + actual_offset,
				info->sizes[i].translation_times);
		actual_offset += info->sizes[i].translation_times;

		nodes[i].rotations.resize(info->sizes[i].rotations / sizeof(glm::quat));
		memcpy(nodes[i].rotations.data(), decompressed_buffer.data() + actual_offset, info->sizes[i].rotations);
		actual_offset += info->sizes[i].rotations;

		nodes[i].rotation_times.resize(info->sizes[i].rotation_times / sizeof(float));
		memcpy(nodes[i].rotation_times.data(), decompressed_buffer.data() + actual_offset,
				info->sizes[i].rotation_times);
		actual_offset += info->sizes[i].rotation_times;
		// I've got a bug that appears in some models, that last value of rotation_times was wrong after load but
		// before save it was correct
		float z = 0;
		memcpy(&z, decompressed_buffer.data() + (actual_offset - 4), sizeof(float));
		if (z != info->duration_seconds * 1000) {
			nodes[i].rotation_times.pop_back();
			nodes[i].rotation_times.push_back(info->duration_seconds * 1000);
		}
		//		for (auto &n : nodes[i].rotation_times) {
		//			SPDLOG_INFO("{}", n);
		//		}
	}
}
