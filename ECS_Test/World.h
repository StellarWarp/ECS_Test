#pragma once
#include "ISystem.h"
#include "EntityManager.h"

class World
{
	std::vector<ISystem*> m_systems;

	EntityManager m_entityManager;

public:

	void Update();

	EntityManager& GetEntityManager() { return m_entityManager; }


};

