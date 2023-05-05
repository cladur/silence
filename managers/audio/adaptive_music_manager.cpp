#include "adaptive_music_manager.h"
#include "audio_manager.h"
#include "fmod_errors.h"

#define FMOD_CHECK(x)                                                                                                  \
	do {                                                                                                               \
		FMOD_RESULT result = x;                                                                                        \
		if (result != FMOD_OK) {                                                                                       \
			SPDLOG_ERROR("Audio Manager: FMOD error! {}", result, FMOD_ErrorString(result));                           \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)


//AutoCVarFloat music_drum_intensity("music.drum_intensity", "drum intensity volume" , 0.0f);
//AutoCVarFloat music_intensity("music.intensity", "intensity volume" , 0.0f);
//AutoCVarFloat music_lpf("music.lpf", "low pass filter" , 0.0f);

AutoCVarFloat music_detection_level("music.detection_level", "detection level volume" , 0.0f);
AutoCVarInt music_enemy_near_count("music.enemy_near_count", "number of enemies near" , 1);
AutoCVarInt music_is_sprinting("music.is_sprinting", "is sprinting" , 0);
AutoCVarInt music_is_crounching("music.is_crounching", "is crounching" , 0);

void AdaptiveMusicManager::startup(std::string name_of_event) {
	event_ref = EventReference(name_of_event);
	event_instance = AudioManager::get().create_event_instance(event_ref);
}

void AdaptiveMusicManager::shutdown() {
	event_instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	event_instance->release();
}

void AdaptiveMusicManager::update(float dt) {

	// some made up parameters for controlling the music
	int enemy_near_count = music_enemy_near_count.get();
	float detection_level = music_detection_level.get();
	float detection_to_intensity;
	if (detection_level > 0.99f) {
		detection_to_intensity = 1.0f;
	} else {
		detection_to_intensity = 0.0f;
	}
	int is_sprinting = music_is_sprinting.get();
	int is_crouching = music_is_crounching.get();

	// goes from 0 at 0 to 0.67 at inf or 1.0 if detected
	float drum_intensity = (-(5.0f / ((float)enemy_near_count + 5.0f)) + 1.0f) * 0.67f + detection_to_intensity * 0.33f;

	// melodic intensity based on number of enemies, sprinting, crounching and detection
	float melodic_intensity = (float)is_sprinting * 0.15f + detection_to_intensity * 0.5f + (float)is_crouching * -0.1f + std::clamp((float)enemy_near_count * 0.05f, 0.0f, 1.0f) * 0.55f;

	// slightly filter out the music if the player is crouching
	float lp_filter = 1.0f - (float)is_crouching * 0.2f;

	// just to make sure
	drum_intensity = std::clamp(drum_intensity, 0.0f, 1.0f);
	melodic_intensity = std::clamp(melodic_intensity, 0.0f, 1.0f);

	AudioManager::get().set_global_param_by_name(detection_param, detection_level);
	AudioManager::get().set_global_param_by_name(drum_param, drum_intensity);
	AudioManager::get().set_global_param_by_name(intensity_param, melodic_intensity);
	AudioManager::get().set_global_param_by_name(lpf, lp_filter);
}

void AdaptiveMusicManager::play() {
	event_instance->start();
}

void AdaptiveMusicManager::stop() {
	event_instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}

AdaptiveMusicManager &AdaptiveMusicManager::get() {
	static AdaptiveMusicManager instance;
	return instance;
}
