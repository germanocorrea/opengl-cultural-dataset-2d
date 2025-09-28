//
// Created by gegebc on 28/09/2025.
//

#ifndef CG_TD1_ENTITYPOSITION_H
#define CG_TD1_ENTITYPOSITION_H
#include "Types.h"


class EntityPosition {
public:
	Position position;
	long unsigned int frame;
	Position interpolateWith(const EntityPosition& other, const float t);

private:
	int interpolate(const int a, const int b, const float t);
};


#endif //CG_TD1_ENTITYPOSITION_H