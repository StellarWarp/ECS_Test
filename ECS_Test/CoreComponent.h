#pragma once
#include "ICompomemt.h"
#include "Math.h"

struct Transform : public IComponent
{
	float3 position;
	quaternion rotation;
	float3 scale;
};

struct PhysicsMass : public IComponent
{
	float mass;

};

namespace
{
	static TypeRegister reg_Transform(
		&Transform::position,
		&Transform::rotation,
		&Transform::scale);

	static TypeRegister reg_PhysicsMass(
		&PhysicsMass::mass);
}


