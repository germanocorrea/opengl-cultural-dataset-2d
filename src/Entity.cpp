//
// Created by gegebc on 28/09/2025.
//

#include "Entity.h"

#include <cmath>

Entity::Entity(const int id, std::vector<EntityPosition> positions, const int max_frames) {
	this->id = id;
	this->positions = std::move(positions);
	this->max_frames = max_frames;
}

Entity::Entity(const int id, const bool isControllable, const Position overridePosition) {
	this->id = id;
	this->isControllable = isControllable;
	this->overridePosition = overridePosition;
}

void Entity::updateCurrentFrame() {
	if ((going_backwards && (current_entity_frame > 0)) || current_entity_frame >= max_frames) {
		going_backwards = true;
		current_entity_frame -= 0.1;
	} else {
		going_backwards = false;
		current_entity_frame += 0.1;
	}
}

Position Entity::getEntityPosition() {
	if (isControllable) {
		return {
			.x = overridePosition.x,
			.y = overridePosition.y,
		};
	}

	const size_t floored_frame = normalizeIdx(std::floor(current_entity_frame), positions.size());
	const size_t ceiled_frame = normalizeIdx(std::ceil(current_entity_frame), positions.size());
	if (current_entity_frame - floored_frame > 0.0) {
		return positions[floored_frame].interpolateWith(
			positions[ceiled_frame],
			current_entity_frame - floored_frame
		);
	}

	const size_t idx = normalizeIdx(current_entity_frame, positions.size());
	return positions[idx].position;
}

size_t Entity::normalizeIdx(const int idx, const int count) {
	return std::min<size_t>(static_cast<size_t>(idx), count - 1);
}