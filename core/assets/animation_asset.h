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
	std::vector<BufferSizes> sizes;
	std::vector<std::string> node_names;
	size_t full_size;
	float duration_seconds;
	std::string original_file;
};

AnimationInfo read_animation_info(AssetFile *file);
assets::AssetFile pack_animation(AnimationInfo *info, std::vector<assets::NodeAnimation> &nodes);
void unpack_animation(AnimationInfo *info, const char *source_buffer, std::vector<assets::NodeAnimation> &nodes);

} //namespace assets

#endif //SILENCE_ANIMATION_ASSET_H
