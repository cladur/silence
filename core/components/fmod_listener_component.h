#ifndef SILENCE_FMOD_LISTENER_COMPONENT_H
#define SILENCE_FMOD_LISTENER_COMPONENT_H

struct FmodListener {
	int listener_id;
	glm::vec3 prev_frame_position{};
};

#endif //SILENCE_FMOD_LISTENER_COMPONENT_H
