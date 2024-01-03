#pragma once
#include "Reflection.h"
#include "Entity.h"

struct IComponent
{
};

struct IResource
{
};

template<typename T>
struct is_component : std::is_base_of<IComponent, T> {};

template<typename T>
struct is_component_ref : is_component<typename std::remove_reference<T>::type> {};

template<typename T>
struct is_resource : std::is_base_of<IResource, T> {};

template<typename T>
struct is_resource_ref : is_resource<typename std::remove_reference<T>::type> {};




