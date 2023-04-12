#ifndef SILENCE_COMPONENT_VISITOR_H
#define SILENCE_COMPONENT_VISITOR_H

#include "components/children_component.h"
#include "components/fmod_listener_component.h"
#include "components/gravity_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"
#include "ecs_manager.h"
#include "types.h"

#define UPDATECOMPONENT ecs_manager.update_component(entity, component);
extern ECSManager ecs_manager;

class ComponentVisitor {
private:
	void visit(Entity entity, Children &component) {
		UPDATECOMPONENT
	}
	void visit(Entity entity, Parent &component) {
		UPDATECOMPONENT
	}
	void visit(Entity entity, Transform &component) {
		UPDATECOMPONENT
	}
	void visit(Entity entity, RigidBody &component) {
		UPDATECOMPONENT
	}
	void visit(Entity entity, FmodListener &component) {
		UPDATECOMPONENT
	}
	void visit(Entity entity, Gravity &component) {
		UPDATECOMPONENT
	}
	void visit(Entity entity, MeshInstance &component) {
		UPDATECOMPONENT
	}
    void visit(Entity entity, UIText &component) {
        UPDATECOMPONENT
    }

public:
	static void visit(Entity entity, serialization::variant_type &variant) {
		switch (variant.index()) {
			case 0: {
				Children &component = std::get<0>(variant);
				if (!ecs_manager.has_component<Children>(entity)) {
					ecs_manager.add_component<Children>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
			case 1: {
				Parent &component = std::get<1>(variant);
				if (!ecs_manager.has_component<Parent>(entity)) {
					ecs_manager.add_component<Parent>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
			case 2: {
				Transform &component = std::get<2>(variant);
				if (!ecs_manager.has_component<Transform>(entity)) {
					ecs_manager.add_component<Transform>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
			case 3: {
				RigidBody &component = std::get<3>(variant);
				if (!ecs_manager.has_component<RigidBody>(entity)) {
					ecs_manager.add_component<RigidBody>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
			case 4: {
				FmodListener &component = std::get<4>(variant);
				if (!ecs_manager.has_component<FmodListener>(entity)) {
					ecs_manager.add_component<FmodListener>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
			case 5: {
				Gravity &component = std::get<5>(variant);
				if (!ecs_manager.has_component<Gravity>(entity)) {
					ecs_manager.add_component<Gravity>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
			case 6: {
				MeshInstance &component = std::get<6>(variant);
				if (!ecs_manager.has_component<MeshInstance>(entity)) {
					ecs_manager.add_component<MeshInstance>(entity, component);
				}
				ComponentVisitor().visit(entity, component);
				break;
			}
            case 7: {
                UIText &component = std::get<7>(variant);
                if (!ecs_manager.has_component<UIText>(entity)) {
                    ecs_manager.add_component<UIText>(entity, component);
                }
                ComponentVisitor().visit(entity, component);
                break;
            }
		}
	}
};

#endif //SILENCE_COMPONENT_VISITOR_H
