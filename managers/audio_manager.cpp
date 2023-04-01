#include "audio_manager.h"

#include "fmod_errors.h"

#define FMOD_CHECK(x)                                                                                                  \
	do {                                                                                                               \
		FMOD_RESULT result = x;                                                                                        \
		if (result != FMOD_OK) {                                                                                       \
			SPDLOG_ERROR("FMOD error! {}", result, FMOD_ErrorString(result));                                          \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

void AudioManager::startup() {
	FMOD_CHECK(FMOD::Studio::System::create(&system));
	FMOD_CHECK(system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr));

	SPDLOG_INFO("Initialized audio manager");
}

void AudioManager::shutdown() {
	FMOD_CHECK(system->release());
}

void AudioManager::update() {
	FMOD_CHECK(system->update());
}
