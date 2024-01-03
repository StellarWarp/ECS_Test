#pragma once
#include "Lib.h"


struct Entity
{
	uint32_t id;
	uint32_t version;

	Entity() : id(0), version(0) {}
	Entity(uint32_t id, uint32_t version) : id(id), version(version) {}

	bool operator==(const Entity& other) const
	{
		return id == other.id && version == other.version;
	}
};

template<typename T>
struct is_entity : std::is_same<T, Entity> {};

namespace std {
	template <> //function-template-specialization
	struct hash<Entity> {
		size_t operator()(const Entity& val) const
		{
			return *reinterpret_cast<const uint64_t*>(&val);
		}
	};
};