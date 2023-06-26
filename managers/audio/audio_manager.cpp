#include "audio_manager.h"
#include "fmod_errors.h"
#include <components/transform_component.h>
#include <engine/scene.h>
#include <gameplay/gameplay_manager.h>

#define FMOD_CHECK(x)                                                                                                  \
	do {                                                                                                               \
		FMOD_RESULT result = x;                                                                                        \
		if (result != FMOD_OK) {                                                                                       \
			SPDLOG_ERROR("Audio Manager: FMOD error! {} {}", result, FMOD_ErrorString(result));                        \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

AutoCVarInt cvar_enable_audio("audio.enabled", "is audio enabled", 0, CVarFlags::EditCheckbox);

AudioManager &AudioManager::get() {
	static AudioManager instance;
	return instance;
}

void AudioManager::startup() {
	FMOD_CHECK(FMOD::Studio::System::create(&system));
	FMOD_CHECK(system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr));
	load_startup_banks();
	FMOD_CHECK(system->setNumListeners(2));

	event_paths = get_all_event_paths();

	system->getBus("bus:/", &master_bus);

	SPDLOG_INFO("Audio Manager: Initialized audio manager");
}

void AudioManager::update(Scene &scene) {
	ZoneScopedNC("AudioManager::update", 0xcacaca);
	FMOD_CHECK(system->update());
	static bool audio_enabled = !cvar_enable_audio.get();

	if (audio_enabled != cvar_enable_audio.get()) {
		audio_enabled = cvar_enable_audio.get();
		if (!audio_enabled) {
			master_bus->setVolume(0.0f);
		} else {
			master_bus->setVolume(1.0f);
		}
	}
	this->scene = &scene;
	auto &gm = GameplayManager::get();
	auto &agent_tf = scene.world.get_component<Transform>(gm.get_agent_camera(&scene));
	set_3d_listener_attributes(SILENCE_FMOD_LISTENER_AGENT, agent_tf.get_global_position(),
			glm::vec3(0.0f), agent_tf.get_global_forward(), agent_tf.get_global_up());

	auto &hacker_tf = scene.world.get_component<Transform>(gm.get_hacker_camera(&scene));
	set_3d_listener_attributes(SILENCE_FMOD_LISTENER_HACKER, hacker_tf.get_global_position(),
			glm::vec3(0.0f), hacker_tf.get_global_forward(), hacker_tf.get_global_up());

	for (auto *instance : event_instances) {
		instance->setListenerMask(create_listener_mask(instance));
	}
}

void AudioManager::shutdown() {
	SPDLOG_INFO("Audio Manager: Shutting down audio manager, unloading banks");
	FMOD_CHECK(master_bank->unloadSampleData());
	FMOD_CHECK(strings_bank->unloadSampleData());

	FMOD_CHECK(master_bank->unload());
	FMOD_CHECK(strings_bank->unload());

	for (auto &bank : banks) {
		FMOD_CHECK(bank->unloadSampleData());
		FMOD_CHECK(bank->unload());
	}

	FMOD_CHECK(system->unloadAll());
	FMOD_CHECK(system->release());
	SPDLOG_INFO("Audio Manager: Audio Manager shutdown");
}

void AudioManager::load_startup_banks() {
	FMOD_CHECK(system->loadBankFile(
			(path_to_banks + "Master" + bank_postfix).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &master_bank));
	SPDLOG_INFO("Audio Manager: Loaded bank: {}", (path_to_banks + "Master" + bank_postfix).c_str());
	FMOD_CHECK(system->loadBankFile(
			(path_to_banks + "Master.strings" + bank_postfix).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &strings_bank));
	SPDLOG_INFO("Audio Manager: Loaded bank: {}", (path_to_banks + "Master.strings" + bank_postfix).c_str());
	load_bank("SFX");
	load_bank("Music");
	load_bank("Ambience");
}

void AudioManager::load_bank(const std::string &name) {
	FMOD::Studio::Bank *bank = nullptr;
	FMOD_CHECK(
			system->loadBankFile((path_to_banks + name + bank_postfix).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
	SPDLOG_INFO("Audio Manager: Loaded bank: {}", name.c_str());
	banks.push_back(bank);
}

void AudioManager::load_sample_data() {
	SPDLOG_INFO("Audio Manager: Loading sample data");
	FMOD_CHECK(master_bank->loadSampleData());
	FMOD_CHECK(strings_bank->loadSampleData());

	for (auto &bank : banks) {
		FMOD_CHECK(bank->loadSampleData());
	}
}

void AudioManager::test_play_sound() {
	FMOD_CHECK(test_event_instance->start());
}

FMOD::Studio::EventInstance *AudioManager::create_event_instance(const EventReference &event_ref) {
	FMOD::Studio::EventDescription *event_description = nullptr;
	FMOD_RESULT res = system->getEvent((event_path_prefix + event_ref.path).c_str(), &event_description);
	if (res != FMOD_OK) {
		SPDLOG_ERROR(
				"Audio Manager: Failed to get event description for {}. {}", event_ref.path, FMOD_ErrorString(res));
		return nullptr;
	}

	FMOD::Studio::EventInstance *event_instance = nullptr;
	FMOD_CHECK(event_description->createInstance(&event_instance));

	return event_instance;
}

void AudioManager::play_one_shot_2d(const EventReference &event_ref) {
	FMOD::Studio::EventInstance *event_instance = create_event_instance(event_ref);
	if (event_instance == nullptr) {
		return;
	}
	FMOD_CHECK(event_instance->start());
	FMOD_CHECK(event_instance->release());
}

void AudioManager::set_3d_listener_attributes(
		int listener_id, glm::vec3 position, glm::vec3 velocity, glm::vec3 forward, glm::vec3 up) {
	FMOD_3D_ATTRIBUTES attributes;
	attributes.position = { position.x, position.y, position.z };
	attributes.velocity = { velocity.x, velocity.y, velocity.z };
	attributes.forward = { forward.x, forward.y, forward.z };
	attributes.up = { up.x, up.y, up.z };
	FMOD_CHECK(system->setListenerAttributes(listener_id, &attributes));
}

void AudioManager::play_one_shot_3d(const EventReference &event_ref, Transform &transform, RigidBody *rigid_body) {
	auto event_instance = create_event_instance(event_ref);
	FMOD_3D_ATTRIBUTES attributes = to_3d_attributes(transform, rigid_body);
	FMOD_CHECK(event_instance->set3DAttributes(&attributes));
	FMOD_CHECK(event_instance->setListenerMask(create_listener_mask(event_instance)));
	FMOD_CHECK(event_instance->start());
	FMOD_CHECK(event_instance->release());
}

FMOD_3D_ATTRIBUTES AudioManager::to_3d_attributes(Transform &transform, RigidBody *rigid_body) {
	FMOD_3D_ATTRIBUTES attributes = {};
	auto p = transform.get_global_position();
	auto f = transform.get_global_forward();
	auto u = transform.get_global_up();
	attributes.position = to_fmod_vector(p);
	attributes.forward = to_fmod_vector(f);
	attributes.up = to_fmod_vector(u);
	if (rigid_body != nullptr) {
		attributes.velocity = { rigid_body->velocity.x, rigid_body->velocity.y, rigid_body->velocity.z };
	}
	return attributes;
}

FMOD_GUID AudioManager::path_to_guid(const std::string &path) {
	FMOD_GUID guid;
	FMOD_RESULT res = system->lookupID((event_path_prefix + path).c_str(), &guid);
	if (res != FMOD_OK) {
		SPDLOG_ERROR("Audio Manager: Failed to get guid for {}. {}", path, FMOD_ErrorString(res));
		return FMOD_GUID{ 0, 0, 0, "0000000" };
	}
	return guid;
}

FMOD::Studio::System *AudioManager::get_system() {
	return system;
}

FMOD::Studio::EventInstance *AudioManager::create_event_instance(const std::string &event_name) {
	EventReference event_ref = EventReference(event_name);

	FMOD::Studio::EventDescription *event_description = nullptr;
	FMOD_RESULT res = system->getEvent((event_path_prefix + event_ref.path).c_str(), &event_description);
	if (res != FMOD_OK) {
		SPDLOG_ERROR(
				"Audio Manager: Failed to get event description for {}. {}", event_ref.path, FMOD_ErrorString(res));
		return nullptr;
	}

	FMOD::Studio::EventInstance *event_instance = nullptr;
	FMOD_CHECK(event_description->createInstance(&event_instance));

	return event_instance;
}

bool AudioManager::set_global_param_by_name(const std::string &name, float value) {
	float val;
	FMOD_RESULT res = system->getParameterByName(name.c_str(), &val);
	if (res != FMOD_OK) {
		SPDLOG_ERROR("Audio Manager: Failed to get parameter {}. {}", name, FMOD_ErrorString(res));
		return false;
	}
	FMOD_CHECK(system->setParameterByName(name.c_str(), value));
	return true;
}

std::vector<std::string> AudioManager::get_all_event_paths() {
	// get event paths from all banks
	std::vector<std::string> event_paths;
	for (auto &bank : banks) {
		int count = 0;
		FMOD_CHECK(bank->getEventCount(&count));
		if (count == 0) {
			continue;
		}
		std::vector<FMOD::Studio::EventDescription *> event_descriptions(count);
		FMOD_CHECK(bank->getEventList(&event_descriptions[0], count, &count));
		for (auto &event_description : event_descriptions) {
			char path[256];
			FMOD_CHECK(event_description->getPath(path, 256, nullptr));
			event_paths.emplace_back(path);
		}
	}
	int count = 0;
	FMOD_CHECK(master_bank->getEventCount(&count));
	if (count == 0) {
		return event_paths;
	}
	std::vector<FMOD::Studio::EventDescription *> event_descriptions(count);
	FMOD_CHECK(master_bank->getEventList(&event_descriptions[0], count, &count));
	for (auto &event_description : event_descriptions) {
		char path[256];
		FMOD_CHECK(event_description->getPath(path, 256, nullptr));
		event_paths.emplace_back(path);
	}

	return event_paths;
}

bool AudioManager::is_valid_event_path(const std::string &path) {
	std::string full_path = AudioManager::get().event_path_prefix + path;
	return std::any_of(event_paths.begin(), event_paths.end(), [&full_path](const std::string &event_path) {
		return event_path == full_path;
	});
}

FMOD_VECTOR AudioManager::to_fmod_vector(glm::vec3 vector) {
	FMOD_VECTOR temp;
	temp.x = vector.x;
	temp.y = vector.y;
	temp.z = vector.z;
	return temp;
}

void AudioManager::play_local(FMOD::Studio::EventInstance *instance) {
	FMOD_CHECK(instance->start());
	event_instances.emplace_back(instance);
}

uint32_t AudioManager::create_listener_mask(FMOD::Studio::EventInstance *instance) {
	auto &gm = GameplayManager::get();
	auto &agent_tf = scene->world.get_component<Transform>(gm.get_agent_camera(scene));
	auto &hacker_tf = scene->world.get_component<Transform>(gm.get_hacker_camera(scene));

	FMOD_3D_ATTRIBUTES attributes;
	instance->get3DAttributes(&attributes);
	glm::vec3 instance_pos = glm::vec3(attributes.position.x, attributes.position.y, attributes.position.z);
	float dist_to_agent = glm::distance2(instance_pos, agent_tf.get_global_position());
	float dist_to_hacker = glm::distance2(instance_pos, hacker_tf.get_global_position());
	if (dist_to_agent < dist_to_hacker) {
		return 1 << SILENCE_FMOD_LISTENER_AGENT;
	} else {
		return 1 << SILENCE_FMOD_LISTENER_HACKER;
	}
}

void AudioManager::stop_local(FMOD::Studio::EventInstance *instance) {
	FMOD_CHECK(instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
	event_instances.erase(std::remove(event_instances.begin(), event_instances.end(), instance),
			event_instances.end());
}

void AudioManager::play_one_shot_3d_with_params(const EventReference &event_ref, Transform &transform,
		RigidBody *rigid_body, std::vector<std::pair<std::string, float>> params) {
	FMOD::Studio::EventInstance *instance = create_event_instance(event_ref.path);
	if (instance == nullptr) {
		return;
	}

	FMOD_3D_ATTRIBUTES attributes;
	attributes.position = to_fmod_vector(transform.get_global_position());
	attributes.forward = to_fmod_vector(transform.get_forward());
	attributes.up = to_fmod_vector(transform.get_up());
	if (rigid_body != nullptr) {
		attributes.velocity = to_fmod_vector(rigid_body->velocity);
	} else {
		attributes.velocity = {0, 0, 0};
	}
	FMOD_CHECK(instance->set3DAttributes(&attributes));

	for (auto &param : params) {
		FMOD_CHECK(instance->setParameterByName(param.first.c_str(), param.second));
	}

	FMOD_CHECK(instance->start());
	FMOD_CHECK(instance->release());
}