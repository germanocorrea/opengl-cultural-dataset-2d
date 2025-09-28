//
// Created by gegebc on 28/09/2025.
//

#ifndef CG_TD1_ENTITYPOSITION_H
#define CG_TD1_ENTITYPOSITION_H
#include "Types.h"


class EntityPosition {
public:
	int entity_id;
	Position position{};
	long unsigned int frame;

	EntityPosition(int entity_id, Position position, long unsigned int frame);
	Position interpolateWith(const EntityPosition& other, const float t) const;

private:
	static int interpolate(const int a, const int b, const float t);
};


#endif //CG_TD1_ENTITYPOSITION_H