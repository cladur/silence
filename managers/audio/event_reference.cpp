#include "event_reference.h"
#include "audio_manager.h"

EventReference::EventReference(const std::string &path) {
	this->path = path;
	guid = AudioManager::get().path_to_guid(path);
}

const std::string &EventReference::get_path() const {
	return path;
}

const FMOD_GUID &EventReference::get_guid() const {
	return guid;
}

std::string EventReference::guid_to_string() const {
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	ss << std::setw(8) << guid.Data1 << "-";
	ss << std::setw(4) << guid.Data2 << "-";
	ss << std::setw(4) << guid.Data3 << "-";
	ss << std::setw(2) << guid.Data4[0];
	ss << std::setw(2) << guid.Data4[1];
	ss << "-";
	ss << std::setw(2) << guid.Data4[2];
	ss << std::setw(2) << guid.Data4[3];
	ss << std::setw(2) << guid.Data4[4];
	ss << std::setw(2) << guid.Data4[5];
	ss << std::setw(2) << guid.Data4[6];
	ss << std::setw(2) << guid.Data4[7];
	std::string buffer = ss.str();
	return std::string(buffer);
}
