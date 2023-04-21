#include "cvars.h"

#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include <shared_mutex>

enum class CVarType : char {
	INT,
	FLOAT,
	STRING,
};

class CVarParameter {
public:
	friend class CVarSystemImpl;

	int32_t array_index;

	CVarType type;
	CVarFlags flags;
	std::string name;
	std::string description;
};

template <typename T> struct CVarStorage {
	T initial;
	T current;
	CVarParameter *parameter = nullptr;
};

template <typename T> struct CVarArray {
	CVarStorage<T> *cvars;
	int32_t last_cvar{ 0 };

	explicit CVarArray(size_t size) {
		cvars = new CVarStorage<T>[size]();
	}

	~CVarArray() {
		// If we delete cvars, it says that the pointer was not allocated???
		// delete cvars;
	}

	T get_current(int32_t index) {
		return cvars[index].current;
	};

	T *get_current_ptr(int32_t index) {
		return &cvars[index].current;
	};

	void set_current(const T &val, int32_t index) {
		cvars[index].current = val;
	}

	int add(const T &value, CVarParameter *param) {
		int index = last_cvar;

		cvars[index].current = value;
		cvars[index].initial = value;
		cvars[index].parameter = param;

		param->array_index = index;
		last_cvar++;
		return index;
	}

	int add(const T &initial_value, const T &current_value, CVarParameter *param) {
		int index = last_cvar;

		cvars[index].current = current_value;
		cvars[index].initial = initial_value;
		cvars[index].parameter = param;

		param->array_index = index;
		last_cvar++;

		return index;
	}
};

class CVarSystemImpl : public CVarSystem {
public:
	constexpr static int MAX_INT_CVARS = 1000;
	CVarArray<int32_t> int_cvars{ MAX_INT_CVARS };

	constexpr static int MAX_FLOAT_CVARS = 1000;
	CVarArray<float> float_cvars{ MAX_FLOAT_CVARS };

	constexpr static int MAX_STRING_CVARS = 200;
	CVarArray<std::string> string_cvars{ MAX_STRING_CVARS };

	//using templates with specializations to get the cvar arrays for each type.
	//if you try to use a type that doesn't have specialization, it will trigger a linked error
	template <typename T> CVarArray<T> *get_cvar_array();

	template <> CVarArray<int32_t> *get_cvar_array() {
		return &int_cvars;
	}
	template <> CVarArray<float> *get_cvar_array() {
		return &float_cvars;
	}
	template <> CVarArray<std::string> *get_cvar_array() {
		return &string_cvars;
	}

	CVarParameter *get_cvar(string_utils::StringHash hash) final;

	CVarParameter *create_float_cvar(
			const char *name, const char *description, float default_value, float current_value) final;
	CVarParameter *create_int_cvar(
			const char *name, const char *description, int32_t default_value, int32_t current_value) final;
	CVarParameter *create_string_cvar(
			const char *name, const char *description, const char *default_value, const char *current_value) final;

	float *get_float_cvar(string_utils::StringHash hash) final;
	int32_t *get_int_cvar(string_utils::StringHash hash) final;
	std::string *get_string_cvar(string_utils::StringHash hash) final;

	void set_float_cvar(string_utils::StringHash hash, float value) final;
	void set_int_cvar(string_utils::StringHash hash, int32_t value) final;
	void set_string_cvar(string_utils::StringHash hash, std::string &&value) final;

	//templated get-set cvar versions for syntax sugar
	template <typename T> T *get_cvar_current(uint32_t namehash) {
		CVarParameter *par = get_cvar(namehash);
		if (!par) {
			return nullptr;
		} else {
			return get_cvar_array<T>()->get_current_ptr(par->array_index);
		}
	}

	template <typename T> void set_cvar_current(uint32_t namehash, const T &value) {
		CVarParameter *cvar = get_cvar(namehash);
		if (cvar) {
			get_cvar_array<T>()->set_current(value, cvar->array_index);
		}
	}

	static CVarSystemImpl *get() {
		return dynamic_cast<CVarSystemImpl *>(CVarSystem::get());
	}

	void draw_imgui_editor() final;
	void edit_parameter(CVarParameter *p, float text_width);

private:
	std::shared_mutex mutex;

	CVarParameter *init_cvar(const char *name, const char *description);

	std::unordered_map<uint32_t, CVarParameter> saved_cvars;

	std::vector<CVarParameter *> cached_edit_parameters;
};

CVarParameter *CVarSystemImpl::get_cvar(string_utils::StringHash hash) {
	std::shared_lock lock(mutex);

	auto it = saved_cvars.find(hash);

	if (it != saved_cvars.end()) {
		return &(*it).second;
	}

	return nullptr;
}

CVarParameter *CVarSystemImpl::init_cvar(const char *name, const char *description) {
	if (get_cvar(name)) {
		return nullptr; //return null if the cvar already exists
	}

	uint32_t namehash = string_utils::StringHash{ name };
	saved_cvars[namehash] = CVarParameter{};

	CVarParameter &new_param = saved_cvars[namehash];

	new_param.name = name;
	new_param.description = description;

	return &new_param;
}

CVarParameter *CVarSystemImpl::create_float_cvar(
		const char *name, const char *description, float default_value, float current_value) {
	std::shared_lock lock(mutex);

	CVarParameter *param = init_cvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CVarType::FLOAT;

	get_cvar_array<float>()->add(default_value, current_value, param);

	return param;
}

CVarParameter *CVarSystemImpl::create_int_cvar(
		const char *name, const char *description, int32_t default_value, int32_t current_value) {
	std::shared_lock lock(mutex);

	CVarParameter *param = init_cvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CVarType::INT;

	get_cvar_array<int32_t>()->add(default_value, current_value, param);

	return param;
}

CVarParameter *CVarSystemImpl::create_string_cvar(
		const char *name, const char *description, const char *default_value, const char *current_value) {
	std::shared_lock lock(mutex);

	CVarParameter *param = init_cvar(name, description);
	if (!param) {
		return nullptr;
	}

	param->type = CVarType::STRING;

	get_cvar_array<std::string>()->add(default_value, current_value, param);

	return param;
}

float *CVarSystemImpl::get_float_cvar(string_utils::StringHash hash) {
	return get_cvar_current<float>(hash);
}

int32_t *CVarSystemImpl::get_int_cvar(string_utils::StringHash hash) {
	return get_cvar_current<int32_t>(hash);
}

std::string *CVarSystemImpl::get_string_cvar(string_utils::StringHash hash) {
	return get_cvar_current<std::string>(hash);
}

void CVarSystemImpl::set_float_cvar(string_utils::StringHash hash, float value) {
	set_cvar_current<float>(hash, value);
}

void CVarSystemImpl::set_int_cvar(string_utils::StringHash hash, int32_t value) {
	set_cvar_current<int32_t>(hash, value);
}

void CVarSystemImpl::set_string_cvar(string_utils::StringHash hash, std::string &&value) {
	set_cvar_current<std::string>(hash, value);
}

//static initialized singleton pattern
CVarSystem *CVarSystem::get() {
	static CVarSystemImpl cvar_system{};
	return &cvar_system;
}

//get the cvar data purely by type and array index
template <typename T> T get_cvar_current_by_index(int32_t index) {
	return CVarSystemImpl::get()->get_cvar_array<T>()->get_current(index);
}

template <typename T> T *ptr_get_cvar_current_by_index(int32_t index) {
	return CVarSystemImpl::get()->get_cvar_array<T>()->get_current_ptr(index);
}

//set the cvar data purely by type and index
template <typename T> void set_cvar_current_by_index(int32_t index, const T &data) {
	CVarSystemImpl::get()->get_cvar_array<T>()->set_current(data, index);
}

AutoCVarFloat::AutoCVarFloat(const char *name, const char *description, float default_value, CVarFlags flags) {
	CVarParameter *cvar = CVarSystem::get()->create_float_cvar(name, description, default_value, default_value);
	cvar->flags = flags;
	index = cvar->array_index;
}

float AutoCVarFloat::get() {
	return get_cvar_current_by_index<CVarType>(index);
}

float *AutoCVarFloat::get_ptr() {
	return ptr_get_cvar_current_by_index<CVarType>(index);
}

void AutoCVarFloat::set(float val) {
	set_cvar_current_by_index<CVarType>(index, val);
}

AutoCVarInt::AutoCVarInt(const char *name, const char *description, int32_t default_value, CVarFlags flags) {
	CVarParameter *cvar = CVarSystem::get()->create_int_cvar(name, description, default_value, default_value);
	cvar->flags = flags;
	index = cvar->array_index;
}

int32_t AutoCVarInt::get() {
	return get_cvar_current_by_index<CVarType>(index);
}

int32_t *AutoCVarInt::get_ptr() {
	return ptr_get_cvar_current_by_index<CVarType>(index);
}

void AutoCVarInt::set(int32_t val) {
	set_cvar_current_by_index<CVarType>(index, val);
}

void AutoCVarInt::toggle() {
	bool enabled = get() != 0;

	set(enabled ? 0 : 1);
}

AutoCVarString::AutoCVarString(const char *name, const char *description, const char *default_value, CVarFlags flags) {
	CVarParameter *cvar = CVarSystem::get()->create_string_cvar(name, description, default_value, default_value);
	cvar->flags = flags;
	index = cvar->array_index;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
const char *AutoCVarString::get() {
	return get_cvar_current_by_index<CVarType>(index).c_str();
}
#pragma clang diagnostic pop

void AutoCVarString::set(std::string &&val) {
	set_cvar_current_by_index<CVarType>(index, val);
}

void CVarSystemImpl::draw_imgui_editor() {
	static std::string search_text;

	ImGui::Begin("CVar Editor");

	ImGui::InputText("Filter", &search_text);
	static bool b_show_advanced = false;
	ImGui::Checkbox("Advanced", &b_show_advanced);
	ImGui::Separator();
	cached_edit_parameters.clear();

	auto add_to_edit_list = [&](auto parameter) {
		bool b_hidden = ((uint32_t)parameter->flags & (uint32_t)CVarFlags::Noedit);
		bool b_is_advanced = ((uint32_t)parameter->flags & (uint32_t)CVarFlags::Advanced);

		if (!b_hidden) {
			if (!(!b_show_advanced && b_is_advanced) && parameter->name.find(search_text) != std::string::npos) {
				cached_edit_parameters.push_back(parameter);
			};
		}
	};

	for (int i = 0; i < get_cvar_array<int32_t>()->last_cvar; i++) {
		add_to_edit_list(get_cvar_array<int32_t>()->cvars[i].parameter);
	}
	for (int i = 0; i < get_cvar_array<float>()->last_cvar; i++) {
		add_to_edit_list(get_cvar_array<float>()->cvars[i].parameter);
	}
	for (int i = 0; i < get_cvar_array<std::string>()->last_cvar; i++) {
		add_to_edit_list(get_cvar_array<std::string>()->cvars[i].parameter);
	}

	if (cached_edit_parameters.size() > 10) {
		std::unordered_map<std::string, std::vector<CVarParameter *>> categorized_params;

		//insert all the edit parameters into the hashmap by category
		for (auto p : cached_edit_parameters) {
			int dot_pos = -1;
			//find where the first dot is to categorize
			for (int i = 0; i < p->name.length(); i++) {
				if (p->name[i] == '.') {
					dot_pos = i;
					break;
				}
			}
			std::string category;
			if (dot_pos != -1) {
				category = p->name.substr(0, dot_pos);
			}

			auto it = categorized_params.find(category);
			if (it == categorized_params.end()) {
				categorized_params[category] = std::vector<CVarParameter *>();
				it = categorized_params.find(category);
			}
			it->second.push_back(p);
		}

		for (auto [category, parameters] : categorized_params) {
			//alphabetical sort
			std::sort(parameters.begin(), parameters.end(),
					[](CVarParameter *A, CVarParameter *B) { return A->name < B->name; });

			if (ImGui::BeginMenu(category.c_str())) {
				float max_text_width = 0;

				for (auto p : parameters) {
					max_text_width = std::max(max_text_width, ImGui::CalcTextSize(p->name.c_str()).x);
				}
				for (auto p : parameters) {
					edit_parameter(p, max_text_width);
				}

				ImGui::EndMenu();
			}
		}
	} else {
		//alphabetical sort
		std::sort(cached_edit_parameters.begin(), cached_edit_parameters.end(),
				[](CVarParameter *A, CVarParameter *B) { return A->name < B->name; });
		float max_text_width = 0;
		for (auto p : cached_edit_parameters) {
			max_text_width = std::max(max_text_width, ImGui::CalcTextSize(p->name.c_str()).x);
		}
		for (auto p : cached_edit_parameters) {
			edit_parameter(p, max_text_width);
		}
	}

	ImGui::End();
}

void label(const char *label, float text_width) {
	constexpr float slack = 50;
	constexpr float editor_width = 100;

	ImGuiWindow *window = ImGui::GetCurrentWindow();
	const ImVec2 line_start = ImGui::GetCursorScreenPos();
	const ImGuiStyle &style = ImGui::GetStyle();
	float full_width = text_width + slack;

	ImVec2 text_size = ImGui::CalcTextSize(label);

	ImVec2 start_pos = ImGui::GetCursorScreenPos();

	ImGui::Text("%s", label);

	ImVec2 final_pos = { start_pos.x + full_width, start_pos.y };

	ImGui::SameLine();
	ImGui::SetCursorScreenPos(final_pos);

	ImGui::SetNextItemWidth(editor_width);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
void CVarSystemImpl::edit_parameter(CVarParameter *p, float text_width) {
	const bool readonly_flag = ((uint32_t)p->flags & (uint32_t)CVarFlags::EditReadOnly);
	const bool checkbox_flag = ((uint32_t)p->flags & (uint32_t)CVarFlags::EditCheckbox);
	const bool drag_flag = ((uint32_t)p->flags & (uint32_t)CVarFlags::EditFloatDrag);

	switch (p->type) {
		case CVarType::INT:

			if (readonly_flag) {
				std::string display_format = p->name + "= %i";
				ImGui::Text(display_format.c_str(), get_cvar_array<int32_t>()->get_current(p->array_index));
			} else {
				if (checkbox_flag) {
					bool b_checkbox = get_cvar_array<int32_t>()->get_current(p->array_index) != 0;
					label(p->name.c_str(), text_width);

					ImGui::PushID(p->name.c_str());

					if (ImGui::Checkbox("", &b_checkbox)) {
						get_cvar_array<int32_t>()->set_current(b_checkbox ? 1 : 0, p->array_index);
					}
					ImGui::PopID();
				} else {
					label(p->name.c_str(), text_width);
					ImGui::PushID(p->name.c_str());
					ImGui::InputInt("", get_cvar_array<int32_t>()->get_current_ptr(p->array_index));
					ImGui::PopID();
				}
			}
			break;

		case CVarType::FLOAT:

			if (readonly_flag) {
				std::string display_format = p->name + "= %f";
				ImGui::Text(display_format.c_str(), get_cvar_array<float>()->get_current(p->array_index));
			} else {
				label(p->name.c_str(), text_width);
				ImGui::PushID(p->name.c_str());
				if (drag_flag) {
					ImGui::InputFloat("", get_cvar_array<float>()->get_current_ptr(p->array_index), 0, 0, "%.3f");
				} else {
					ImGui::InputFloat("", get_cvar_array<float>()->get_current_ptr(p->array_index), 0, 0, "%.3f");
				}
				ImGui::PopID();
			}
			break;

		case CVarType::STRING:

			if (readonly_flag) {
				std::string display_format = p->name + "= %s";
				ImGui::PushID(p->name.c_str());
				ImGui::Text(display_format.c_str(), get_cvar_array<std::string>()->get_current(p->array_index));

				ImGui::PopID();
			} else {
				label(p->name.c_str(), text_width);
				ImGui::InputText("", get_cvar_array<std::string>()->get_current_ptr(p->array_index));

				ImGui::PopID();
			}
			break;

		default:
			break;
	}

	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("%s", p->description.c_str());
	}
}
#pragma clang diagnostic pop