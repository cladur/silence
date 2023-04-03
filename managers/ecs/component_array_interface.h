#ifndef SILENCE_ICOMPONENTARRAY_H
#define SILENCE_ICOMPONENTARRAY_H

#include <memory>
class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	virtual void entity_destroyed(Entity entity) = 0;
	virtual bool has_component(Entity entity) = 0;
	virtual void serialize_entity(nlohmann::json &j, Entity entity) = 0;
};

#endif //SILENCE_ICOMPONENTARRAY_H
