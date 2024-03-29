#pragma once
#include "ICompomemt.h"
#include "Query.h"
#include "CoreComponent.h"

struct CommandBuffer
{

};


class EntityManager
{
	std::queue<Entity> m_freeEntities;


	std::vector<SortKey> m_entitySortKeys;

	std::vector<ArchtypeArray*> m_archTypes;//component data
	std::unordered_map<Archtype, uint32_t> m_archTypeMap;//archtype index
	std::vector<IQuery*> m_queries;





	void Initialize()
	{
		m_entitySortKeys.reserve(2048);
	}

public:
	EntityManager()
	{
		Initialize();
	}

	~EntityManager()
	{
		for (auto archtype : m_archTypes)
		{
			delete archtype;
		}
	}

private:

	Entity AllocateEntity()
	{
		if (m_freeEntities.empty())
		{
			m_entitySortKeys.emplace_back(0, 0);
			return Entity{ (uint32_t)(m_entitySortKeys.size() - 1), 1 };
		}
		else
		{
			Entity entity = m_freeEntities.front();
			m_freeEntities.pop();
			entity.version++;
			return entity;
		}
	}

	bool CreateArchtpe(Archtype archtype)
	{
		//find if archtype exist
		auto it = m_archTypeMap.find(archtype);
		if (it != m_archTypeMap.end()) return false;
		//create new archtype
		uint32_t archtypeIndex = m_archTypes.size();
		m_archTypes.push_back(new ArchtypeArray(archtype, archtypeIndex));
		m_archTypeMap[archtype] = archtypeIndex;
		// update all query
		for (auto query : m_queries)
		{
			query->TryMatch(m_archTypes.back());
		}

		return true;
	}

public:
	Entity Instantiate(Archtype archtype)
	{
		Entity entity = AllocateEntity();

		CreateArchtpe(archtype);
		//get archtype index
		uint32_t archtypeIndex = m_archTypeMap[archtype];
		//insert entity to archtype and get arrayIndex
		SortKey sortKey = m_archTypes[archtypeIndex]->AddEntity(entity);

		m_entitySortKeys[entity.id] = sortKey;


		return entity;
	}

	void Destroy(Entity entity)
	{
		//get archtype
		SortKey sortKey = m_entitySortKeys[entity.id];
		ArchtypeArray* archtype = m_archTypes[sortKey.archetypeIndex];
		//remove entity from archtype
		archtype->RemoveEntity(sortKey.arrayIndex);
		//add entity to free list
		m_freeEntities.push(entity);
		//update entity sort key
		//m_entitySortKeys[entity.id] = { 0,0 };
		//update all query
		//for (auto query : m_queries)
		//{
		//	query.TryMatch(&archtype);
		//}



	}



public:

	template<class T>
	T& GetComponentRW(Entity entity)
	{
		static std::remove_reference_t<T> test{};
		return test;
	}

	template<class T>
	const T& GetComponentRO(Entity entity)
	{
		return T();
	}

	template<class T>requires std::is_base_of_v<IComponent, T> ||
		std::is_base_of_v<IResource, T> ||
		std::is_same_v<T, Entity>
		T InfoAccess()
	{
		if constexpr (std::is_same_v<T, Entity>)
		{
			return Entity();
		}
		else if constexpr (std::is_base_of_v<IComponent, T>)
		{
			if constexpr (std::is_reference<T>)
			{
				return GetComponentRW<T>(Entity());

			}
			else
			{
				return GetComponentRO<T>(Entity());
			}

		}
		else if constexpr (std::is_base_of_v<IResource, T>)
		{

		}
	}

	template<class T>
	T& GetResourse()
	{
		return T();
	}

public:
	template<typename T>
	void RegisterQuery(T& query)
	{
		m_queries.push_back(&query);
		for (auto archtype : m_archTypes)
		{
			query.TryMatch(archtype);
		}
	}



	/// iterate api
private:

	template<typename... Ts, typename... Tuples>
	constexpr auto BuildArgs(type_list<Ts...>, Tuples... tuples)
	{
		static_assert(type_list<Ts...>::is_unique, "type_list must be unique");
		static_assert(type_list<>::from_tuple<Tuples...>::template contains<Ts...>, "type_list must be in tuple");

		return std::tuple<Ts...>(std::get<Ts>(std::tuple_cat(tuples...))...);
	}

	template<typename... Ts>
	constexpr auto BuildResourceTuple(type_list<Ts...> resource_list)
	{
		return std::make_tuple(GetResourse<Ts>()...);
	}

	template<typename U>
	using raw_component_to_const_ref_t = std::conditional_t<
		is_component<U>::value,
		std::add_lvalue_reference_t<std::add_const_t<U>>,
		U
	>;

public:
	template<typename Callable>
	void ProcessEntities(Callable f)//auto query version
	{
		using args_list = function_traits<Callable>::args::template modify_with<raw_component_to_const_ref_t>;
		using component_list = args_list::template filter_with<is_component_ref>::raw_type;
		using access_list = args_list::template filter_with<is_accessable>;
		using resource_list = args_list::template filter_with<is_resource>;

		//print_type(args_list{});

		static Query<
			component_list,
			type_list<>,
			type_list<>,
			component_list> query(
				[this](decltype(query)& q) {
					RegisterQuery(q);
					std::cout << typeid(q).name() << std::endl;
				});

		auto resource_tuple = BuildResourceTuple(resource_list{});

		query.ForEach([&](const access_list::tuple_t& acc_t) {
			auto args_tuple = BuildArgs(args_list{}, resource_tuple, acc_t);
			std::apply(f, args_tuple);
			});

	}
};

////test
//static EntityManager g_entityManager;
//
//struct TestSystem
//{
//
//	TestSystem()
//	{
//		g_entityManager.ProcessEntities([](Entity e, Transform t, SortKey sortKey) {
//			//do something
//			});
//
//
//		Query<
//			type_list<Transform, PhysicsMass>,
//			type_list<>,
//			type_list<>,
//			type_list<Transform>> query;
//
//
//
//		for (auto [e, k, t] : query.GetAccessor<Entity, SortKey, Transform>())
//		{
//		};
//
//		ArchTypeArray archtypeArray1(ArchType{ typeid(Transform) }, 0);
//
//		archtypeArray1.GetComponentOffset<Transform>();
//
//		ArchTypeArray archtypeArray(ArchType{ typeid(Transform),typeid(PhysicsMass) }, 0);
//
//
//		auto offset = archtypeArray.GetComponentOffset<Transform, PhysicsMass>();
//		archtypeArray.ForEach([&](const std::tuple<Transform&>& t) {
//			//do something
//			}, offset);
//
//
//		QueryBuilder builder;
//
//		auto res = builder.All<Transform>().Build();
//	}
//
//};
//
//static TestSystem g_testSystem;
