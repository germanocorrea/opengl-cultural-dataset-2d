#ifndef CG_TD1_ENTITY_H
#define CG_TD1_ENTITY_H
#include <cstddef>
#include <vector>
#include "EntityPosition.h"
#include "Types.h"


class Entity {
public:
	int id;
	bool isControllable = false;
	Position overridePosition{};
	std::vector<EntityPosition *> positions;
	Color color = {0.2, 0.8, 0};
	int max_frames = 0;
	float current_entity_frame = 0;
	bool going_backwards = false;
	EntityPosition* current_position;

	Entity(int id, std::vector<EntityPosition* > positions, int max_frames);
	Entity(int id, bool isControllable, Position overridePosition);
	void updateCurrentFrame();
	EntityPosition* getCurrentPosition();
	~Entity();

private:
	std::size_t normalizeIdx(const int idx, const int count);
};


#endif //CG_TD1_ENTITY_H