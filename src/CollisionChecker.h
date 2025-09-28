#ifndef CG_TD1_COLLISIONCHECKER_H
#define CG_TD1_COLLISIONCHECKER_H
#include <vector>
#include "Entity.h"
#include "EntityPosition.h"

struct EntityPairs {
	Entity *first;
	Entity *second;
	bool collision_detected;
};

class CollisionChecker {
public:
	CollisionChecker(std::vector<EntityPosition *> current_positions, std::vector<Entity *> entities, int entity_side_size);
	std::vector<Entity *> getCollidingEntities();
private:
	std::vector<Entity *> entities;
	std::vector<EntityPairs> pairs;
};


#endif //CG_TD1_COLLISIONCHECKER_H