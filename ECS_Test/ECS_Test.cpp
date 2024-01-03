#include "pch.h"
#include "World.h"
#include "QueryBuilder.h"

struct ComponentA : public IComponent
{
	float3 f1;
	float3 f2;
};

struct ComponentB : public IComponent
{
	int b;
};

struct ComponentC : public IComponent
{
	int* c;
	ComponentC() { c = new int[1]; }
	~ComponentC() { delete[] c; }
};

extern __declspec(selectany) TypeRegister regA(&ComponentA::f1, &ComponentA::f2);
extern __declspec(selectany) TypeRegister regB(&ComponentB::b);
extern __declspec(selectany) TypeRegister regC(&ComponentC::c);

template<typename T>
using type_to_string = std::string;

int main()
{
	World world;
	auto& entityManager = world.GetEntityManager();

	for (size_t i = 0; i < 10; i++)
		entityManager.Instantiate({ typeid(ComponentA) });
	for (size_t i = 0; i < 10; i++)
		entityManager.Instantiate({ typeid(ComponentA),typeid(ComponentB) });
	for (size_t i = 0; i < 10; i++)
		entityManager.Instantiate({ typeid(ComponentB),typeid(ComponentC) });
	for (size_t i = 0; i < 10; i++)
		entityManager.Instantiate({ typeid(ComponentA),typeid(ComponentB),typeid(ComponentC) });

	//inexplicit query generated
	entityManager.ProcessEntities(
		[](const ComponentA& a, SortKey sortKey, ComponentB& b, Entity entity) {
		});

	entityManager.ProcessEntities(
		[](const ComponentA& a, const ComponentB& b, Entity entity, const ComponentC& c) {
		});

	//explicit query
	auto query = QueryBuilder()
		.All<ComponentA, ComponentB>()
		.None<ComponentC>()
		.Build(entityManager);

	query->ForEach([](std::tuple<const ComponentA&, Entity, ComponentB&> t) {
		auto& [a, e, b] = t;
		});
	//iterator version
	for (auto [a, e, b] : query->GetAccessor<ComponentA, Entity, ComponentB&>()) {//reflection print
		std::cout << std::format("entity:{}\n a:{}\n b:{}\n", e.id, std::to_string(a), std::to_string(b));
	}
	//reflection
	auto& info = TypeInfoOf<ComponentA>();
	std::cout << std::to_string(info);
}