#ifndef ANALYZER_UTILITY_HPP_
#define ANALYZER_UTILITY_HPP_

#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/at.hpp>
#include <tuple>
#include <map>

#include "constant_def.hpp"
#include "tuple_utility.hpp"

/**
 * @struct at
 * @brief boost::mpl::at 多加地三個參數功能，可以在取某 key type不存在時，回傳第三個參數的型別
 * @tparam SEQUENCE boost::mpl 內所提及的 sequence
 * @tparam KEY 取得 boost::mpl sequence key 的 value type
 * @tparam DEFAULT 此為如果要取的 key type 不存在而回傳的 type
 */
template<class SEQUENCE, class KEY, class DEFAULT = boost::mpl::void_>
struct at
{
	typedef typename boost::mpl::at<SEQUENCE, KEY>::type tmp_type;
	typedef typename boost::mpl::if_ < boost::is_same<tmp_type, boost::mpl::void_>, DEFAULT, tmp_type >::type type;
};

template<int N, class ...ARGS>
struct tuple_add
{
	static void add(std::tuple<ARGS...> &a, std::tuple<ARGS...> &b)
	{
		std::get<N> (a) = std::get<N>(a) + std::get<N>(b);
		tuple_add<N-1, ARGS...>::add(a, b);
	}
};

template<class ...ARGS>
struct tuple_add< 0 ,ARGS...>
{
	static void add(std::tuple<ARGS...> &a, std::tuple<ARGS...> &b)
	{
		std::get<0> (a) = std::get<0>(a) + std::get<0>(b);
	}
}
;
template<class ...ARGS>
std::tuple<ARGS...> operator+(std::tuple<ARGS...> &a, std::tuple<ARGS...> &b)
{
	//std::get<0> (a) = std::get<0>(a) + std::get<0>(b);
	tuple_add< std::tuple_size < std::tuple<ARGS...> >::value-1 , ARGS...>::add(a, b);
	
	return a;	
};


template<class ...ARGS>
std::map<ARGS...>& operator+=(std::map<ARGS...> &a, std::map<ARGS...> &b)
{
	for(auto &b_v : b)
		a[b_v.first] =  a[b_v.first] + b_v.second;
	return a;
}

template<class ...ARGS>
std::map<ARGS...> operator+(std::map<ARGS...> &a, std::map<ARGS...> &b)
{
	for(auto &b_v : b)
		a[b_v.first] =  a[b_v.first] + b_v.second;
	return a;
}

#endif
