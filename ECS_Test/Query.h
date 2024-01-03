#pragma once
#include "Entity.h"
#include "ArchtypeArray.h"

//template<typename ...T>
//struct TypeList {};

template <typename...T>
struct IQueryConfig : type_list<T...> {};

template <typename...T>
struct IQueryFilter : IQueryConfig<T...> {};

template<typename ...T>
struct QueryFilterAll :IQueryFilter<T...> {};
template<typename ...T>
struct QueryFilterAny : IQueryFilter<T...> {};
template<typename ...T>
struct QueryFilterNone : IQueryFilter<T...> {};
template<typename ...T>
struct QueryAccess : IQueryConfig<T...> {};


struct IQuery
{
	virtual void TryMatch(ArchtypeArray* archTypeArray) {}
};

template<typename T>
struct is_accessable {
	static constexpr bool value = is_component_ref<T>::value || std::is_same_v<T, Entity> || std::is_same_v<T, SortKey>;
};

template<typename AllList, typename AnyList, typename NoneList, typename AccList>
class Query;


template<typename... All, typename... Any, typename... None, typename... Acc>
	requires (is_component<All>::value && ...)
&& (is_component<Any>::value && ...)
&& (is_component<None>::value && ...)
&& (is_accessable<Acc>::value && ...)
struct Query<
	type_list<All...>,
	type_list<Any...>,
	type_list<None...>,
	type_list<Acc...>> : IQuery
{
	struct ArchTypeAccessInfo
	{
		ArchtypeArray* Archtype;
		ArchtypeArray::component_offset_t<Acc...> ComponentOffset;
	};

	std::vector<ArchTypeAccessInfo> m_ArchTypeAccessInfo;


	std::array<type_hash, sizeof...(All)> allComponents = { typeid(All)... };
	std::array<type_hash, sizeof...(Any)> anyComponents = { typeid(Any)... };
	std::array<type_hash, sizeof...(None)> noneComponents = { typeid(None)... };

	Query()
	{
		std::sort(allComponents.begin(), allComponents.end());
		std::sort(anyComponents.begin(), anyComponents.end());
		std::sort(noneComponents.begin(), noneComponents.end());
	}

	template<typename RegisterFunc>
	Query(RegisterFunc registerFunc) : Query()
	{
		registerFunc(*this);
	}

	~Query()
	{
	}

private:
	template<typename U>
	using raw_component_to_const_ref_t = std::conditional_t<
		is_component<U>::value,
		std::add_lvalue_reference_t<std::add_const_t<U>>,
		U
	>;
public:

	template<typename Callable>
	void ForEach(Callable f)
	{
		for (auto& archTypeAccessInfo : m_ArchTypeAccessInfo)
		{
			auto archType = archTypeAccessInfo.Archtype;
			auto componentOffset = archTypeAccessInfo.ComponentOffset;

			archType->ForEach(f, componentOffset);
		}
	}

	template<typename ...T>
	struct Accessor
	{
		Query& query;
		Accessor(Query& query) : query(query) {}

		struct end_iterator {};

		struct iterator
		{
		private:
			iterator()
				:archTypeAccessInfo(*(std::vector<ArchTypeAccessInfo>*)nullptr),
				archTypeIndex((uint32_t)(-1)),
				archTypeAccessor(archtype_accessor_t::empty()),
				archTypeIterator(archtype_accessor_t::iterator::empty())
			{

			}
		public:
			static iterator empty() { return {}; };

		private:
			std::vector<ArchTypeAccessInfo>& archTypeAccessInfo;
			int archTypeIndex;

			using archtype_accessor_t = ArchtypeArray::ForwardAccessor<type_list<T...>, type_list<Acc...>>;
			archtype_accessor_t archTypeAccessor;
			archtype_accessor_t::iterator archTypeIterator;

			constexpr auto access_info()
			{
				return archTypeAccessInfo[archTypeIndex];
			}

			constexpr archtype_accessor_t archtype_accessor()
			{
				return access_info().Archtype->
					GetAccessor<type_list<T...>, type_list<Acc...>>(access_info().ComponentOffset);
			}

			bool is_end()
			{
				return archTypeIndex == (uint32_t)(-1) || archTypeIndex == archTypeAccessInfo.size();
			}

		public:



			iterator(std::vector<ArchTypeAccessInfo>& info) :
				archTypeAccessInfo(info),
				archTypeIndex(0),
				archTypeAccessor(archtype_accessor()),
				archTypeIterator(archTypeAccessor.begin())
			{

			}



			iterator& operator++()
			{
				++archTypeIterator;
				if (archTypeIterator == archTypeAccessor.end())
				{
					archTypeIndex++;
					if (!is_end())
					{
						archTypeAccessor = archtype_accessor();
						archTypeIterator = archTypeAccessor.begin();
					}
				}
				return *this;
			}

			bool operator==(const iterator& other)
			{
				return archTypeIterator == other.archTypeIterator &&
					archTypeIndex == other.archTypeIndex &&
					archTypeIterator == other.archTypeIterator;
			}

			bool operator!=(const iterator& other)
			{
				return !(*this == other);
			}

			bool operator==(const end_iterator& other)
			{
				return is_end();
			}

			bool operator!=(const end_iterator& other)
			{
				return !is_end();
			}

			operator bool()
			{
			}

			std::tuple<T...> operator*()
			{
				assert(!is_end());
				return *archTypeIterator;
			}



		};

		iterator begin()
		{
			if (query.m_ArchTypeAccessInfo.size() == 0)
				return iterator::empty();
			return iterator(query.m_ArchTypeAccessInfo);
		}

		end_iterator end()
		{
			return end_iterator();
		}
	};


	template<typename ...T>
	constexpr auto GetAccessor()
	{

		return Accessor<raw_component_to_const_ref_t<T>...>(*this);
	}




private:
	bool IsMatch(Archtype archType)
	{
		auto archTypeComponents = archType.Components();
		if constexpr (sizeof...(All) > 0)
		{
			auto iter = archTypeComponents.begin();
			for (auto& all : allComponents)
			{
				while (iter != archTypeComponents.end() && *iter < all)
					iter++;
				if (iter == archTypeComponents.end() || *iter != all)
					return false;
			}
		}
		if constexpr (sizeof...(Any) > 0)
		{
			auto iter = archTypeComponents.begin();
			for (auto& any : anyComponents)
			{
				while (iter != archTypeComponents.end() && *iter < any)
					iter++;
				if (iter != archTypeComponents.end() && *iter == any)
					break;
				if (iter == archTypeComponents.end()) return false;
			}
		}
		if constexpr (sizeof...(None) > 0)
		{
			auto iter = archTypeComponents.begin();
			for (auto& none : noneComponents)
			{
				while (iter != archTypeComponents.end() && *iter < none)
					iter++;
				if (iter != archTypeComponents.end() && *iter == none)
					return false;
			}
		}
		return true;
	}

public:
	virtual void TryMatch(ArchtypeArray* archTypeArray) override
	{
		if (IsMatch(archTypeArray->GetArchType()))
		{
			if constexpr (sizeof...(Acc) != 0)
			{
				m_ArchTypeAccessInfo.emplace_back(
					archTypeArray,
					archTypeArray->GetComponentOffset<Acc...>()
				);
			}
		}
	}
};






