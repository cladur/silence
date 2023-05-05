#ifndef SILENCE_ANIMATION_ASSET_H
#define SILENCE_ANIMATION_ASSET_H

#include "asset_loader.h"
namespace assets {

struct NodeAnimation {
	std::vector<std::array<float, 3>> translations;
	std::vector<std::array<float, 4>> rotations;
	std::vector<float> translation_times;
	std::vector<float> rotation_times;
};

struct BufferSizes {
	uint64_t translations;
	uint64_t translation_times;
	uint64_t rotations;
	uint64_t rotation_times;
};

struct AnimationInfo {
	std::unordered_map<uint64_t, BufferSizes> sizes;
	std::unordered_map<uint64_t, std::string> node_names;
	size_t full_size;
	std::string original_file;
};

assets::AssetFile pack_animation(AnimationInfo *info, char *node_data);
//void unpack_mesh(AnimationInfo *info, const char *source_buffer, size_t source_size, char *node_data);

} //namespace assets

#endif //SILENCE_ANIMATION_ASSET_H
