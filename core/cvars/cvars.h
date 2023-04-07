#ifndef SILENCE_CVARS_H
#define SILENCE_CVARS_H

#include "string_utils.h"

class CVarParameter;

enum class CVarFlags : uint32_t {
	None = 0,
	Noedit = 1 << 1,
	EditReadOnly = 1 << 2,
	Advanced = 1 << 3,

	EditCheckbox = 1 << 8,
	EditFloatDrag = 1 << 9,
};

class CVarSystem {
public:
	virtual CVarParameter *get_cvar(string_utils::StringHash hash) = 0;

	virtual CVarParameter *create_float_cvar(
			const char *name, const char *description, float default_value, float current_value) = 0;
	virtual CVarParameter *create_int_cvar(
			const char *name, const char *description, int32_t default_value, int32_t current_value) = 0;
	virtual CVarParameter *create_string_cvar(
			const char *name, const char *description, const char *default_value, const char *current_value) = 0;

	virtual float *get_float_cvar(string_utils::StringHash hash) = 0;
	virtual int32_t *get_int_cvar(string_utils::StringHash hash) = 0;
	virtual std::string *get_string_cvar(string_utils::StringHash hash) = 0;

	virtual void set_float_cvar(string_utils::StringHash hash, float value) = 0;
	virtual void set_int_cvar(string_utils::StringHash hash, int32_t value) = 0;
	virtual void set_string_cvar(string_utils::StringHash hash, std::string &&value) = 0;

	static CVarSystem *get();

	virtual void draw_imgui_editor() = 0;
};

template <typename T> struct AutoCVar {
protected:
	int index;
	using CVarType = T;
};

struct AutoCVarFloat : AutoCVar<float> {
	AutoCVarFloat(const char *name, const char *description, float default_value, CVarFlags flags = CVarFlags::None);

	float get();
	float *get_ptr();
	void set(float val);
};

struct AutoCVarInt : AutoCVar<int32_t> {
	AutoCVarInt(const char *name, const char *description, int32_t default_value, CVarFlags flags = CVarFlags::None);
	int32_t get();
	int32_t *get_ptr();
	void set(int32_t val);

	void toggle();
};

struct AutoCVarString : AutoCVar<std::string> {
	AutoCVarString(
			const char *name, const char *description, const char *default_value, CVarFlags flags = CVarFlags::None);

	const char *get();
	void set(std::string &&val);
};

#endif //SILENCE_CVARS_H
