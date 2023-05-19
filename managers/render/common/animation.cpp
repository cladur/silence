#include "animation.h"
#include "animation/animation_manager.h"
#include "animation/ecs/animation_instance.h"
#include "render/common/skinned_model.h"
#include "render/ecs/skinned_model_instance.h"
#include "resource/resource_manager.h"

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
		channels.insert(std::make_pair(info.node_names[i], Channel(nodes[i])));
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

void Animation::sample(AnimData &data, Pose &result) {
	ResourceManager &resource_manager = ResourceManager::get();
	const Rig &rig = resource_manager.get_skinned_model(data.model->model_handle).rig;
	result.xfroms.resize(rig.names.size());
	for (int32_t i = 0; i < result.xfroms.size(); ++i) {
		result.xfroms[i].translation = rig.positions[i];
		result.xfroms[i].rotation = rig.rotations[i];

		const auto &it = channels.find(rig.names[i]);
		if (it != channels.end()) {
			Channel &channel = it->second;

			channel.update(data.animation->current_time);
			result.xfroms[i] = channel.local_transform;
		}
	}
}
