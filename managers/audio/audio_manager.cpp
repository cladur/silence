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

void AudioManager::startup() {
	FMOD_CHECK(FMOD::Studio::System::create(&system));
	FMOD_CHECK(system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr));
	load_startup_banks();

	SPDLOG_INFO("Audio Manager: Initialized audio manager");
}

void AudioManager::update() {
	ZoneScopedN("AudioManager::update");
	FMOD_3D_ATTRIBUTES attributes;
	system->getListenerAttributes(0, &attributes);
	FMOD_CHECK(system->update());
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

void AudioManager::test_create_instance() {
	// create an array of EventDescription pointers
	//	FMOD::Studio::EventDescription *event_description[10];
	//	int *count = 0;
	//	master_bank->getEventCount(count);
	//	master_bank->getEventList(&event_description[0], 10, count);
	//
	//	for (auto &event : event_description) {
	//		char path[256];
	//		event->getPath(path, 256, nullptr);
	//		SPDLOG_INFO("Audio Manager: {}", path);
	//	}
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

void AudioManager::play_one_shot_3d(const EventReference &eventRef, glm::vec3 position, RigidBody *rigid_body) {
	auto event_instance = create_event_instance(eventRef);
	FMOD_3D_ATTRIBUTES attributes = to_3d_attributes(position, rigid_body);
	event_instance->set3DAttributes(&attributes);
	event_instance->start();
	event_instance->release();
}

FMOD_3D_ATTRIBUTES AudioManager::to_3d_attributes(glm::vec3 position, RigidBody *rigid_body) {
	FMOD_3D_ATTRIBUTES attributes;
	attributes.position = { position.x, position.y, position.z };
	attributes.forward = { 0, 0, 1 };
	attributes.up = { 0, 1, 0 };
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
		char c[8] = { 0 };
		return FMOD_GUID(0, 0, 0, "0000000");
	}
	return guid;
}
