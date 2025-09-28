#include "EntityPosition.h"

EntityPosition::EntityPosition(int entity_id, Position position) {
	this->entity_id = entity_id;
	this->position = position;
}


EntityPosition *EntityPosition::interpolateWith(const EntityPosition &other, const float t) const {
	return new EntityPosition(this->entity_id, {
		.x = interpolate(position.x, other.position.x, t),
		.y = interpolate(position.y, other.position.y, t),
	});
}

bool EntityPosition::hasCollidedWith(EntityPosition& other, int bouding_box_side) const {
	return (
		this->position.x < other.position.x + bouding_box_side &&
		this->position.x + bouding_box_side > other.position.x &&
		this->position.y < other.position.y + bouding_box_side &&
		this->position.y + bouding_box_side > other.position.y
	);
}

int EntityPosition::interpolate(const int a, const int b, const float t) {
	return a + (b - a) * t;
}