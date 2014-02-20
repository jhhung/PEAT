#ifndef IS_TUPLE_TYPE_HPP
#define IS_TUPLE_TYPE_HPP
#include <tuple>
#include <vector>

/// @class IsTupleType template class
/// @brief served as a simple type verifier indicating a to-be-tested object corresponds to std::tuple types or not
/// @tparam T a type parameter
template < typename... T >
struct IsTupleType 
{
/// @memberof value corresponds to the value of false by default
 	static const bool value = false;
};

/// @brief specialized form of the IsTupleType, with the to-be-tested object specialized as the type of std::tuple<T>
template < typename... T >
struct IsTupleType < std::tuple <T...> > 
{
/// @memberof value corresponds to the value of true 
 	static const bool value = true;
};

#endif
