#pragma once

#include "EntityManager.h"
class ISystem
{
	class Context
	{
		EntityManager* m_entityManager;
	};

	public:

	virtual void Update() = 0;

};

