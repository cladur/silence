#ifndef SILENCE_EVENT_REFERENCE_H
#define SILENCE_EVENT_REFERENCE_H

#include "fmod_studio.hpp"
#include "fmod_common.h"

class EventReference {
public:
	std::string path;
	FMOD_GUID guid = {};

	EventReference() = default;
	explicit EventReference(const std::string &path);

	const std::string &get_path() const;

	const FMOD_GUID &get_guid() const;

	std::string guid_to_string() const;

};

#endif //SILENCE_EVENT_REFERENCE_H
