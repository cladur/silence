#ifndef SILENCE_PCH_H
#define SILENCE_PCH_H

// types
#include <cctype>
#include <cstdint>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <variant>

// containers
#include <array>
#include <bitset>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>

// utilities
#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <ranges>
#include <tuple>
#include <utility>

// third party
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "imgui.h"
#include "magic_enum.hpp"
#include "nlohmann/json.hpp"
#include "omp.h"
#include "spdlog/spdlog.h"
#include "tracy/tracy.hpp"
#include <glm/gtc/matrix_transform.hpp>


// our stuff
#include "cvars/cvars.h"
#include "types.h"

#endif //SILENCE_PCH_H
