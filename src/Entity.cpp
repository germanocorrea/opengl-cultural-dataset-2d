#include "Entity.h"
#include <cmath>
#include <utility>

Entity::Entity(const int id, std::vector<EntityPosition *> positions, const int max_frames) {
	this->id = id;
	this->positions = std::move(positions);
	this->max_frames = max_frames;
	this->current_position = this->positions[0];
}

Entity::Entity(const int id, const bool isControllable, const Position overridePosition) {
	this->id = id;
	this->isControllable = isControllable;
	this->overridePosition = overridePosition;
	this->positions = std::vector<EntityPosition *>();
	this->current_position = new EntityPosition(id, overridePosition);
	this->positions.push_back(this->current_position);
}

void Entity::updateCurrentFrame() {
	if ((going_backwards && (current_entity_frame > 0)) || current_entity_frame >= max_frames) {
		going_backwards = true;
		current_entity_frame -= 0.1;
	} else {
		going_backwards = false;
		current_entity_frame += 0.1;
	}
	this->current_position = nullptr;
}

EntityPosition* Entity::getCurrentPosition() {
	if (this->current_position != nullptr) {
		return this->current_position;
	}
	if (isControllable) {
		this->current_position = new EntityPosition(this->id,{
			.x = overridePosition.x,
			.y = overridePosition.y,
		});
		this->trash_can.push_back(this->current_position);
		return this->current_position;
	}

	const size_t floored_frame = normalizeIdx(std::floor(current_entity_frame), positions.size());
	const size_t ceiled_frame = normalizeIdx(std::ceil(current_entity_frame), positions.size());
	if (current_entity_frame - floored_frame > 0.0) {
		this->current_position = positions[floored_frame]->interpolateWith(
			*positions[ceiled_frame],
			current_entity_frame - floored_frame
		);
		this->trash_can.push_back(this->current_position);
	} else {
		const size_t idx = normalizeIdx(current_entity_frame, positions.size());
		this->current_position = positions[idx];
	}
	return this->current_position;
}

Entity::~Entity() {
	for (auto pos : positions) {
		delete pos;
	}
	for (auto pos : trash_can) {
		delete pos;
	}
};


size_t Entity::normalizeIdx(const int idx, const int count) {
	return std::min<size_t>(static_cast<size_t>(idx), count - 1);
}