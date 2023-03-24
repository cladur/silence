#include "../../core/types.h"

#ifndef SILENCE_ICOMPONENTARRAY_H
#define SILENCE_ICOMPONENTARRAY_H

#endif //SILENCE_ICOMPONENTARRAY_H

class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	virtual void entity_destroyed(Entity entity) = 0;
};
