/// @file tuple_utility.hpp
/// @brief Provide tuple filling and printing tools by means of employing static template metaprogramming pardigms
/// @author C-Salt Corp.
#ifndef TUPLE_UTILITY
#define TUPLE_UTILITY

#include "../include/boost_include.hpp"
#include "../include/gtest_include.hpp"
#include "../include/stdlib_include.hpp"
/// @struct TupleUtility<TUPLE, N>  template struct
/// @brief Include a non-type (int) parameter served as index, and a type parameter indicating output data structure, e.g. in std::tuple format\n Current generic template struct also served as initial and continue conditions for static metaprogramming recursive operation achieved by the current TupleUtility<TUPLE, N> template struct\n Include template member functions of FillTuple and PrintTuple, to respectively handle inserting and printing operations\n Specialized TupleUtility template struct, with its non-type (int) parameter specialized to value of zero\n Current specialized template struct also served as terminal condition for static metaprogramming recursive operation achieved by the current TupleUtility template struct
/// @tparam type parameter of TUPLE, indicating output data structure type, e.g. std::tuple
/// @tparam none-type parameter of int, served as index indicating elements sequence of the output data structure
template<class TUPLE, int N>
struct TupleUtility
{
/// @memberof TupleUtility<TUPLE, N>
/// @brief Achieve operation of inserting an N-1 th element into the output data structure, e.g. in std::tuple format from source oobject of split_temp, in std::vector<std::string> format
/// @tparam target the output data object target, in TUPLE& format
/// @tparam split_temp the input data, in std::vector<STRING>& format, which is defaulted as std::vector<std::string>\nIn our present design, split_temp is an object obtained via boost::split operation
    template <class STRING = std::string>
    static void FillTuple( TUPLE& target, std::vector< STRING >& split_temp )
    {
        TupleUtility<TUPLE, N-1>::FillTuple( target, split_temp );
        std::get<N-1>(target) = boost::lexical_cast< typename std::tuple_element<N-1, TUPLE>::type > ( split_temp[N-1] ); 
    }
/// @memberof TupleUtility<TUPLE, N>
/// @brief Achieve operation of printing an N-1 th element of the output data structure, e.g. in std::tuple format, into an ostream object 
/// @param O the ostream object, which is called by reference
/// @param target the output data object target, in TUPLE& format
//  static void PrintTuple(TUPLE target)
	static void PrintTuple(std::ostream& O, const TUPLE& target)
	{
		TupleUtility<TUPLE, N-1>::PrintTuple(O, target);
        O << std::get<N-1> (target) << "\n";
	}
/// @memberof TupleUtility<TUPLE, N>
/// @brief Achieve operation of pushing an N-1 th element of input object target into the output std::string object 
/// @param target the input data object target, in TUPLE& format
/// @param out a std::string object, for holding the pushed element of the target, in std::tuple format
    static void PrintTuple( const TUPLE& target, std::string& out)
    {
        TupleUtility<TUPLE, N-1>::PrintTuple(target, out);
		std::string temp = boost::lexical_cast<std::string>( std::get<N-1>(target) ) ;
		boost::trim_if(temp, boost::is_any_of("\n"));
//		if (temp[temp.length()-1]!='\n')
		if (temp.back()!='\n')	//Proposed by Andy
			out += temp + "\n" ;
		else
			out += temp;
	}

/// @memberof TupleUtility<TUPLE, N>
/// @brief Overloaded version for Bed and Sam, which are tab delimited data structure.
    static void PrintTuple( const TUPLE& target, std::string& out, int i)
    {
        TupleUtility<TUPLE, N-1>::PrintTuple(target, out, i);
		std::string temp = boost::lexical_cast<std::string>( std::get<N-1>(target) ) ;
		boost::trim_if(temp, boost::is_any_of("\n"));
//		if (temp[temp.length()-1]!='\n')
		if (temp.back()!='\n')	//Proposed by Andy
			out += temp + "\t" ;
		else
			out += temp;
	}

	template < typename Archive >
	static void SerializeTuple ( TUPLE& target, Archive& arc, const unsigned int version)
	{
		TupleUtility <TUPLE, N-1> :: SerializeTuple ( target, arc, version );
		arc & std::get<N-1>(target);
	}
};

/// @brief specialized form of the TupleUtility, with the tuple elemnt count of N specialized as the value of 0
template< class TUPLE>
struct TupleUtility< TUPLE, 0 >
{
    template <class STRING = std::string>
/// @memberof TupleUtility<TUPLE, 0>, similar defined as its generic counterpart
    static void FillTuple( TUPLE& target, std::vector<std::string>& split_temp )
    {}
/// @memberof TupleUtility<TUPLE, 0>, similar defined as its generic counterpart
	static void PrintTuple(std::ostream& Out, const TUPLE& target)
	{}
/// @memberof TupleUtility<TUPLE, 0>, similar defined as its generic counterpart
    static void PrintTuple( const TUPLE& target, std::string& out )
	{}
/// @memberof TupleUtility<TUPLE, 0>, similar defined as its generic counterpart
/// @brief Overloaded version for Bed and Sam, which are tab delimited data structure.
    static void PrintTuple( const TUPLE& target, std::string& out, int i )
	{}
	template < typename Archive >
	static void SerializeTuple ( TUPLE& target, Archive& arc, const unsigned int version)
	{
		arc & std::get<0>(target);
	}
};
/**
 * @brief 原型版本，將tuple的內容型別後面在加上一個型別
 */
template<class T, class... ARGS> 
struct tuple_push_front
{};

/**
 * @brief 特化版本，將tuple的內容型別後面在加上一個型別
 */
template<class T, class... ARGS> 
struct tuple_push_front<T, std::tuple<ARGS...> >
{
	typedef std::tuple<T, ARGS...> type;
};


#endif
