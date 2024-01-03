#pragma once

#include "ICompomemt.h"

class Archtype
{
	uint64_t m_id = 0;
	friend struct std::hash<Archtype>;
	std::vector<type_hash> m_ComponentTypes;

	void InitArchTypeID()
	{
		std::sort(m_ComponentTypes.begin(), m_ComponentTypes.end());

		for (auto& type : m_ComponentTypes)
		{
			m_id = m_id << 8;
			m_id += type;
		}
	}

public:

	template<typename ...T>
	static Archtype MakeArchType()
	{
		return Archtype({ typeid(T)... });
	}

	Archtype() = default;

	Archtype(std::initializer_list<type_hash> components)
	{
		m_ComponentTypes.reserve(components.size());
		for (auto& component : components)
			m_ComponentTypes.push_back(component);

		InitArchTypeID();
	}

	Archtype(const Archtype& base, std::initializer_list<type_hash> components)
	{
		m_ComponentTypes = base.m_ComponentTypes;
		m_ComponentTypes.reserve(components.size() + m_ComponentTypes.size());
		for (auto& component : components)
			m_ComponentTypes.push_back(component);

		InitArchTypeID();
	}

	bool operator==(const Archtype& other) const
	{
		return m_id == other.m_id;
	}


	size_t ColumnSize() const
	{
		size_t size = 0;
		for (auto& type : m_ComponentTypes)
		{
			size += ReflectionManager::Sizeof(type);
		}
		return size;
	}

	const std::vector<type_hash>& Components() const
	{
		return m_ComponentTypes;
	}

	size_t ComponentCount() const
	{
		return m_ComponentTypes.size();
	}

};

namespace std
{
	template<>
	struct hash<Archtype>
	{
		size_t operator()(const Archtype& arch) const
		{
			return arch.m_id;
		}
	};
}

 



