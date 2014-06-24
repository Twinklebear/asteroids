#ifndef PTR_TUPLE_H
#define PTR_TUPLE_H

#include <tuple>
#include <type_traits>

namespace detail {
/*
 * Construct a tuple containing pointer types to all the args
 */
template<typename T, typename... Args>
struct PtrTuple {
	typedef typename PtrTuple<std::tuple<typename std::add_pointer<T>::type>, Args...>::type type;
};
template<typename... Tuple, typename T, typename... Args>
struct PtrTuple<std::tuple<Tuple...>, T, Args...> {
	typedef typename PtrTuple<std::tuple<Tuple..., typename std::add_pointer<T>::type>, Args...>::type type;
};
template<typename... Tuple, typename T>
struct PtrTuple<std::tuple<Tuple...>, T> {
	typedef std::tuple<Tuple..., typename std::add_pointer<T>::type> type;
};
template<typename T>
struct PtrTuple<T> {
	typedef std::tuple<typename std::add_pointer<T>::type> type;
};
}

#endif

