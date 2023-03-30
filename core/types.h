#ifndef SILENCE_TYPES_H
#define SILENCE_TYPES_H

#endif //SILENCE_TYPES_H
#pragma once

#include <bitset>
#include <cstdint>

// ECS

// Entity = id
using Entity = std::uint32_t;
// ComponentType = id
using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;
const Entity MAX_ENTITIES = 5000;
const Entity MAX_CHILDREN = 255;
const Entity MAX_IMGUI_ENTITIES = 50;
