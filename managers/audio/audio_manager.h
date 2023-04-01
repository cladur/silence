#ifndef SILENCE_AUDIO_MANAGER_H
#define SILENCE_AUDIO_MANAGER_H

#include "components/rigidbody_component.h"
#include "fmod_studio.hpp"

class AudioManager {
	FMOD::Studio::System *system = nullptr;

	FMOD::Studio::Bank *master_bank = nullptr;
	FMOD::Studio::Bank *strings_bank = nullptr;

	std::vector<FMOD::Studio::Bank *> banks;

	std::string path_to_banks = "resources/fmod_banks/Desktop/";
	std::string bank_postfix = ".bank";
	std::string event_path_prefix = "event:/";

	FMOD::Studio::EventInstance *test_event_instance = nullptr;
public:
	void startup();
	void shutdown();
	void update();

	void load_startup_banks();
	void load_bank(const std::string &name);

	void set_3d_listener_attributes(glm::vec3 position, glm::vec3 velocity, glm::vec3 forward, glm::vec3 up);

	// this loads ALL SAMPLES FROM ALL BANKS
	// which later may create a lot of memory overhead
	// on later stages it'd be better to pre-load samples used rarely
	// and only load samples used often.
	// full chapter about data loading methods:
	// https://www.fmod.com/docs/2.02/api/studio-guide.html#sample-data-loading
	void load_sample_data();

	void test_create_instance();

	void test_play_sound();

	/**
	 * @brief Creates an event instance from the given path.
	 * The path should be in the format "Bank/path/to/event" without "event:/" prefix.
	 * @param path
	 * @return
	 */
	FMOD::Studio::EventInstance *create_event_instance(const std::string &path);

	void play_one_shot_2d(const std::string &path);

	void play_one_shot_3d(const std::string &path, glm::vec3 position, RigidBody *rigid_body = nullptr);

	FMOD_3D_ATTRIBUTES to_3d_attributes(glm::vec3 position, RigidBody *rigid_body = nullptr);
};


#endif //SILENCE_AUDIO_MANAGER_H

