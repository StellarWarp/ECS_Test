#pragma once
#include "Lib.h"


template <typename ...T>
struct type_list
{
	static constexpr size_t size = sizeof...(T);
	static constexpr bool is_empty = std::bool_constant<sizeof...(T) == 0>::value;

	using tuple_t = std::tuple<T...>;

private:
	template<size_t I, typename...Ts>
	struct get_helper;
	template<>
	struct get_helper<0> { using type = void; };
	template<typename U, typename...Us>
	struct get_helper<0, U, Us...> { using type = U; };
	template<size_t I, typename U, typename...Us>
	struct get_helper<I, U, Us...> { using type = get_helper<I - 1, Us...>::type; };

public:

	template <size_t I>
	using get = get_helper<I, T...>::type;

	using remove_ref = type_list<std::remove_reference_t<T>...>;
	using raw_type = type_list<std::remove_cvref_t<T>...>;

private:
	template <typename U>
	static constexpr bool contains_helper = std::bool_constant<(... || std::is_same_v<T, U>)>::value;
public:

	template <typename... U>
	static constexpr bool contains = std::bool_constant<(... && contains_helper<U>)>::value;

private:

	template<typename... Us>
	struct is_unique_helper;
	template<>
	struct is_unique_helper<> { static constexpr bool value = true; };
	template <typename U, typename... Us>
	struct is_unique_helper<U, Us...> {
		static constexpr bool value = !type_list<Us...>::template contains<U> && is_unique_helper<Us...>::value;
	};
public:

	static constexpr bool is_unique = is_unique_helper<T...>::value;




private:

	template <typename ...Us>
	struct pop_front_helper;
	template <typename U, typename ...Us>
	struct pop_front_helper<U, Us...> { using type = type_list<Us...>; };
	template <>
	struct pop_front_helper<> { using type = type_list<>; };

	template <typename L1, typename L2>
	struct pop_back_helper;
	template <typename... T1s>
	struct pop_back_helper<type_list<T1s...>, type_list<>> { using type = type_list<T1s...>; };
	template <typename... T1s, typename T2>
	struct pop_back_helper<type_list<T1s...>, type_list<T2>> { using type = type_list<T1s...>; };
	template <typename... T1s, typename T2, typename... T2s>
	struct pop_back_helper<type_list<T1s...>, type_list<T2, T2s...>> {
		using type = pop_back_helper< type_list<T1s..., T2>, type_list<T2s...>>::type;
	};

	template <typename... L>
	struct cat_helper;
	template <>
	struct cat_helper<> { using type = type_list<>; };
	template <typename... T1>
	struct cat_helper<type_list<T1...>> { using type = type_list<T1...>; };
	template <typename... T1, typename... T2>
	struct cat_helper<type_list<T1...>, type_list<T2...>> { using type = type_list<T1..., T2...>; };
	template <typename L, typename... Ls>
	struct cat_helper<L, Ls...> { using type = typename cat_helper<L, typename cat_helper<Ls...>::type>::type; };


	template <typename L1, typename L2, size_t Remove>
	struct remove_helper;
	///invalid remove
	//template <typename... T1s, size_t Remove>
	//struct remove_helper<type_list<T1s...>, type_list<>, Remove> {
	//	using type = type_list<>;
	//};
	template <typename... T1s, typename T2, typename... T2s>
	struct remove_helper<type_list<T1s...>, type_list<T2, T2s...>, 0> {
		using type = type_list< T1s..., T2s...>;
	};
	template <typename... T1s, typename T2, typename... T2s, size_t Remove>
	struct remove_helper<type_list<T1s...>, type_list<T2, T2s...>, Remove> {
		using type = remove_helper<type_list<T1s..., T2>, type_list<T2s...>, Remove - 1>::type;
	};
public:


	template <typename... List>
	using cat = cat_helper<type_list<T...>, List...>::type;

	using pop_front = pop_front_helper<T...>::type;

	using pop_back = pop_back_helper<type_list<>, type_list<T...>>::type;

	template <typename... U>
	using push_back = type_list<T..., U...>;

	template <typename... U>
	using push_front = type_list<U..., T...>;

	template<size_t I>
	using remove = remove_helper< type_list<>, type_list<T...>, I>::type;

	//template <size_t I, typename... Result, typename... U>
	//static constexpr auto remove_index(type_list<Result...> result, type_list<U...>) {
	//	if constexpr (sizeof...(U) == 0)
	//		return result;
	//	else {
	//		using first = type_list<U...>::template get<0>;
	//		using poped = type_list<Result...>::pop_front;
	//		if constexpr (sizeof...(Result) == I)
	//			return cat_helper<type_list<Result...>, poped>{};
	//		else
	//			return remove_index<I>(type_list<Result..., first>{}, poped{});
	//	}
	//}
	//template<size_t I>
	//using remove = std::decay_t<decltype(remove_index<I>(type_list<>{}, type_list<T...>{})) > ;

private:
	template<typename U>
	struct from_tuple_helper;
	template<typename... U>
	struct from_tuple_helper<std::tuple<U...>> { using type = type_list<U...>; };

public:
	template<typename... Tuple>
	using from_tuple = cat_helper<typename from_tuple_helper<std::remove_cvref_t<Tuple>>::type...>::type;

private:


	template <template <typename> typename Condition, typename LL>
	struct filter_helper;
	template <template <typename> typename Condition, typename... T1s>
	struct filter_helper<Condition, type_list<type_list<T1s...>, type_list<>>> {
		using type = type_list<T1s...>;
	};

	template <bool C, typename L1, typename L2>
	struct filter_helper_branch;
	template <typename... T1s, typename T2, typename... T2s>
	struct filter_helper_branch <true, type_list<T1s...>, type_list<T2, T2s...>> {
		using type = type_list<type_list<T1s..., T2>, type_list<T2s...>>;
	};
	template <typename... T1s, typename T2, typename... T2s>
	struct filter_helper_branch <false, type_list<T1s...>, type_list<T2, T2s...>> {
		using type = type_list<type_list<T1s...>, type_list<T2s...>>;
	};

	template <template <typename> typename Condition, typename... T1s, typename T2, typename... T2s>
	struct filter_helper<Condition, type_list<type_list<T1s...>, type_list<T2, T2s...>>> {
		using filtered = type_list<T1s...>;
		using unfiltered = type_list<T2, T2s...>;
		using f = filter_helper_branch<Condition<T2>::value, filtered, unfiltered>::type;
		using type = filter_helper<Condition, f>::type;
	};

	//template <template <typename> typename Condition, typename... Result, typename... U>
	//static constexpr auto filter_types(type_list<Result...> result, type_list<U...>) {
	//	if constexpr (sizeof...(U) == 0)
	//		return result;
	//	else
	//	{
	//		using first = type_list<U...>::template get<0>;
	//		using poped = type_list<U...>::pop_front;
	//		if constexpr (Condition<first>{})
	//			return filter_types<Condition>(type_list<Result..., first>{}, poped{});
	//		else
	//			return filter_types<Condition>(result, poped{});
	//	}
	//}

public:
	//template <template <typename> typename Condition>
	//using filter_with = std::decay_t<decltype(filter_types<Condition>(type_list<>{}, type_list<T...>{})) > ;

	template <template <typename> typename Condition>
	using filter_with = filter_helper<Condition, type_list<type_list<>, type_list<T...>>>::type;

	template <template <typename> typename Op>
	using modify_with = type_list<Op<T>...>;

};