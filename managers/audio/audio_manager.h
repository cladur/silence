#ifndef SILENCE_AUDIO_MANAGER_H
#define SILENCE_AUDIO_MANAGER_H

#include "components/rigidbody_component.h"
#include "event_reference.h"
#include "fmod_studio.hpp"

//#define SILENCE_FMOD_LISTENER_DEBUG_CAMERA 0
// Add them when we need them
 #define SILENCE_FMOD_LISTENER_AGENT 0
 #define SILENCE_FMOD_LISTENER_HACKER 1

struct Transform;
struct Scene;

class AudioManager {
	FMOD::Studio::System *system = nullptr;

	FMOD::Studio::Bank *master_bank = nullptr;
	FMOD::Studio::Bank *strings_bank = nullptr;

	std::vector<FMOD::Studio::Bank *> banks;

	FMOD::Studio::EventInstance *test_event_instance = nullptr;

	std::vector<std::string> event_paths;

	std::vector<FMOD::Studio::EventInstance*> event_instances;

	Scene *scene = nullptr;

	FMOD::Studio::Bus *master_bus = nullptr;
public:
	std::string path_to_banks = "resources/fmod_banks/Desktop/";
	std::string bank_postfix = ".bank";
	std::string event_path_prefix = "event:/";

	static AudioManager &get();

	void startup();
	void shutdown();
	void update(Scene &scene);

	FMOD::Studio::System *get_system();

	void load_startup_banks();
	void load_bank(const std::string &name);

	void set_3d_listener_attributes(
			int listener_id, glm::vec3 position, glm::vec3 velocity, glm::vec3 forward, glm::vec3 up);

	// this loads ALL SAMPLES FROM ALL BANKS
	// which later may create a lot of memory overhead
	// on later stages it'd be better to pre-load samples used rarely
	// and only load samples used often.
	// full chapter about data loading methods:
	// https://www.fmod.com/docs/2.02/api/studio-guide.html#sample-data-loading
	void load_sample_data();

	void test_play_sound();

	// path without "event:/" prefix
	FMOD::Studio::EventInstance *create_event_instance(const EventReference &event_ref);

	FMOD::Studio::EventInstance *create_event_instance(const std::string &event_name);

	void play_one_shot_2d(const EventReference &event_ref);

	void play_one_shot_3d(const EventReference &event_ref, Transform &transform, RigidBody *rigid_body = nullptr);

	void play_one_shot_3d_with_params(
			const EventReference &event_ref,
			Transform &transform,
			RigidBody *rigid_body = nullptr,
			std::vector<std::pair<std::string, float>> params = {});

	static FMOD_3D_ATTRIBUTES to_3d_attributes(Transform &transform, RigidBody *rigid_body = nullptr);

	FMOD_GUID path_to_guid(const std::string &path);

	bool set_global_param_by_name(const std::string &name, float value);

	std::vector<std::string> get_all_event_paths();

	bool is_valid_event_path(const std::string &path);

	static FMOD_VECTOR to_fmod_vector(glm::vec3 vector);

	void play_local(FMOD::Studio::EventInstance *instance);

	void stop_local(FMOD::Studio::EventInstance *instance);

	uint32_t create_listener_mask(FMOD::Studio::EventInstance *instance);
};

#endif //SILENCE_AUDIO_MANAGER_H
