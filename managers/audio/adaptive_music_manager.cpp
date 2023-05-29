#include "adaptive_music_manager.h"
#include "audio_manager.h"
#include "fmod_errors.h"
#include <gameplay/gameplay_manager.h>
//#include <render/transparent_elements/ui_manager.h>

#define FMOD_CHECK(x)                                                                                                  \
	do {                                                                                                               \
		FMOD_RESULT result = x;                                                                                        \
		if (result != FMOD_OK) {                                                                                       \
			SPDLOG_ERROR("Audio Manager: FMOD error! {}", result, FMOD_ErrorString(result));                           \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

AutoCVarFloat music_detection_level("music.detection_level", "detection level volume" , 0.0f);
AutoCVarInt music_enemy_near_count("music.enemy_near_count", "number of enemies near" , 1);
AutoCVarInt music_is_sprinting("music.is_sprinting", "is sprinting" , 0);
AutoCVarInt music_is_crounching("music.is_crounching", "is crounching" , 0);

void AdaptiveMusicManager::startup(std::string name_of_event) {
	event_ref = EventReference(name_of_event);
	event_instance = AudioManager::get().create_event_instance(event_ref);

//	auto &ui = UIManager::get();
//	ui.create_ui_scene("adaptive_music");
//	auto &anchor = ui.add_ui_anchor("adaptive_music", "adaptive_music_anchor");
//	anchor.is_screen_space = true;
//	anchor.x = 0.0f;
//	anchor.y = 1.0f;
//	auto &text = ui.add_ui_text("adaptive_music", "drum_intensity");
//	text.text = "drum_intensity: 0%";
//	text.is_screen_space = true;
//	text.size = glm::vec2(0.6f, 0.6f);
//	text.centered_x = false;
//	text.position = glm::vec3(150.0f, -100.0f, 0.0f);
//
//	auto &text1 = ui.add_ui_text("adaptive_music", "melodic_intensity");
//	text1.text = "melodic_intensity: 0%";
//	text1.is_screen_space = true;
//	text1.size = glm::vec2(0.6f, 0.6f);
//	text1.centered_x = false;
//	text1.position = glm::vec3(150.0f, -150.0f, 0.0f);
//
//	auto &text2 = ui.add_ui_text("adaptive_music", "detection");
//	text2.text = "detection: 0%";
//	text2.is_screen_space = true;
//	text2.size = glm::vec2(0.6f, 0.6f);
//	text2.centered_x = false;
//	text2.position = glm::vec3(150.0f, -200.0f, 0.0f);
//
//	ui.add_as_root("adaptive_music", "adaptive_music_anchor");
//	ui.add_to_root("adaptive_music", "drum_intensity", "adaptive_music_anchor");
//	ui.add_to_root("adaptive_music", "melodic_intensity", "adaptive_music_anchor");
//	ui.add_to_root("adaptive_music", "detection", "adaptive_music_anchor");
//	ui.activate_ui_scene("adaptive_music");
}

void AdaptiveMusicManager::shutdown() {
	event_instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	event_instance->release();
}

void AdaptiveMusicManager::update(float dt) {
	auto &gm = GameplayManager::get();
	// some made up parameters for controlling the music
	int enemy_near_count = gm.get_enemies_near_player();	//music_enemy_near_count.get();
	float detection_level = gm.get_highest_detection();	//music_detection_level.get();
	float detection_to_intensity;
	if (detection_level > 0.99f) {
		detection_to_intensity = 1.0f;
	} else {
		detection_to_intensity = 0.0f;
	}
	int is_sprinting = music_is_sprinting.get();
	bool is_crouching = gm.get_agent_crouch(); //music_is_crounching.get();

	// goes from 0 at 0 to 0.8 at inf enemies near without detection. goes to 1.0 if detected
	float drum_intensity = (-(drum_int_calc_val / ((float)enemy_near_count + drum_int_calc_val)) + 1.0f) * 0.8f + detection_to_intensity * 0.2f;
	drum_intensity = drum_intensity * drum_intensity + drum_intensity / 5.0f;

	// melodic intensity based on number of enemies, sprinting, crounching and detection
	float melodic_intensity = (float)is_sprinting * 0.1f + detection_to_intensity * 0.5f + (float)is_crouching * -0.1f + std::clamp((float)enemy_near_count * 0.15f, 0.0f, 1.0f) * 0.55f;

	// slightly filter out the music if the player is crouching
	float lp_filter = 1.0f - (float)is_crouching * 0.2f;

	// just to make sure
	drum_intensity = std::clamp(drum_intensity, 0.0f, 1.0f);
	melodic_intensity = std::clamp(melodic_intensity, 0.0f, 1.0f);

//	auto &ui = UIManager::get();
//	ui.get_ui_text("adaptive_music", "drum_intensity").text = fmt::format("drum_intensity: {:.0f}%", drum_intensity * 100.0f);
//	ui.get_ui_text("adaptive_music", "melodic_intensity").text = fmt::format("melodic_intensity: {:.0f}%", melodic_intensity * 100.0f);
//	ui.get_ui_text("adaptive_music", "detection").text = fmt::format("detection: {:.0f}%", detection_level * 100.0f);

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
