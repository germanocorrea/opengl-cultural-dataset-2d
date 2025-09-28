#include "CollisionChecker.h"
#include "Entity.h"

struct Area {
	Position top_left;
	Position top_right;
	Position bottom_left;
	Position bottom_right;
};

Entity* getEntityFromList(std::vector<Entity*> entities, int id) {
	for (auto entity : entities) {
		if (entity->id == id) {
			return entity;
		}
	}
	return nullptr;
}

bool checkIfAlreadyExists(const std::vector<EntityPairs>& pairs, Entity *first, Entity *second) {
	for (auto pair : pairs) {
		if (pair.first == first && pair.second == second) {
			return true;
		}
		if (pair.first == second && pair.second == first) {
			return true;
		}
	}
	return false;
}

std::vector<Entity *> CollisionChecker::getCollidingEntities() {
	std::vector<Entity *> colliding_entities;
	for (auto pair : pairs) {
		if (pair.collision_detected) {
			colliding_entities.push_back(pair.first);
			colliding_entities.push_back(pair.second);
		}
	}
	return colliding_entities;
}

CollisionChecker::CollisionChecker(
	std::vector<EntityPosition *> current_positions,
	std::vector<Entity *> entities,
	int entity_side_size
) {
	this->pairs.clear();
	this->entities = entities;
	for (int i = 0; i < current_positions.size(); i++) {
		for (int j = 0; j < current_positions.size(); j++) {
			if (i == j) {
				continue;
			}
			EntityPosition *pos1 = current_positions[i];
			EntityPosition *pos2 = current_positions[j];
			Entity* ent1 = getEntityFromList(entities, pos1->entity_id);
			Entity* ent2 = getEntityFromList(entities, pos2->entity_id);

			if (ent1 == nullptr || ent2 == nullptr) {
				continue;
			}
			if (checkIfAlreadyExists(this->pairs, ent1, ent2)) {
				continue;
			}

			this->pairs.push_back({
				.first = ent1,
				.second = ent2,
				.collision_detected = pos1->hasCollidedWith(*pos2, entity_side_size)
			});
		}
	}

}
