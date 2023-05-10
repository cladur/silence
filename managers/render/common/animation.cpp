#include "animation.h"

void Animation::load_from_asset(const char *path) {
	name = path;

	assets::AssetFile file;
	bool loaded = assets::load_binary_file(path, file);
	if (!loaded) {
		SPDLOG_ERROR("Error when loading animation file at path {}", path);
		assert(false);
	} else {
		SPDLOG_INFO("Animation {} loaded to cache", path);
	}

	assets::AnimationInfo info = assets::read_animation_info(&file);

	std::vector<assets::NodeAnimation> nodes;

	assets::unpack_animation(&info, file.binary_blob.data(), nodes);

	channels.reserve(nodes.size());
	for (int32_t i = 0; i < nodes.size(); ++i) {
		channels.emplace_back(nodes[i], info.node_names[i], i);
	}

	ticks_per_second = 1000;
	duration_ms = info.duration_seconds * SECONDS_TO_MS;
}

int32_t Animation::get_ticks_per_second() const {
	return ticks_per_second;
}

float Animation::get_duration() const {
	return duration_ms;
}

Channel *Animation::find_channel(const std::string &bone_name) {
	auto iterator = std::find_if(channels.begin(), channels.end(),
			[&](const Channel &channel) { return channel.get_bone_name() == bone_name; });

	if (iterator == channels.end()) {
		return nullptr;
	} else {
		return &(*iterator);
	}
}
