#ifndef CG_TD1_ENTITYPOSITION_H
#define CG_TD1_ENTITYPOSITION_H
#include "Types.h"

class EntityPosition {
public:
	int entity_id;
	Position position{};

	EntityPosition(int entity_id, Position position);

	EntityPosition *interpolateWith(const EntityPosition &other, const float t) const;
	bool hasCollidedWith(EntityPosition& other, int bouding_box_side) const;

private:
	static int interpolate(const int a, const int b, const float t);
};

#endif //CG_TD1_ENTITYPOSITION_H