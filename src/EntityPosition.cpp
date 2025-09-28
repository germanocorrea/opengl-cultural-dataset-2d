//
// Created by gegebc on 28/09/2025.
//

#include "EntityPosition.h"

Position EntityPosition::interpolateWith(const EntityPosition &other, const float t) const {
	return {
		.x = interpolate(position.x, other.position.x, t),
		.y = interpolate(position.y, other.position.y, t),
	};
}

int EntityPosition::interpolate(const int a, const int b, const float t) {
	return a + (b - a) * t;
}