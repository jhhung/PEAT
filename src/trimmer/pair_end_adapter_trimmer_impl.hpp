/**
 *  @file pair_end_adapter_trimmer_impl.hpp
 *  @brief provide PEAT implementation detail
 *  @author C-Salt Corp.
 */
#ifndef PAIR_END_ADAPTER_TRIMMER_PARAMETER_IMPL_HPP_
#define PAIR_END_ADAPTER_TRIMMER_PARAMETER_IMPL_HPP_
#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <random>
#include <thread>
#include <tuple>
#include <unordered_map>
#include "../str_match/linear_string_match.hpp"
//#include "../thread_pool_update.hpp"
#include "../thread_control_version.hpp"
#include "boost/filesystem.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/device/file_descriptor.hpp"
#include "boost/range.hpp"

/**
 * @brief provide a static type detector, which returns true when the passed in type T is double, and returns false otherwise.
 */
template <typename T>
struct IsInDoubleType 
{
	static const bool value = false;
};

template <>
struct IsInDoubleType <double>
{
	static const bool value = true;
};

/**
 * @brief provide a utility class having an iterator range reversed, so as to facilitate function similar to that of std::reverse().
 */
template <typename RANGE>
inline void RangeIteratorReverse (RANGE itr_begin, RANGE itr_end)
{
	--itr_end;
	while (itr_begin < itr_end)
	{
		std::iter_swap (itr_begin, itr_end);
		++itr_begin;
		--itr_end;
	}
}


/**
 * @class TrimTrait
 * @brief provide a trait to designate the datatype and the match scheme that PEAT is going to apply.
 * @tparam DATATYPE indicating the data type of the incoming FORMAT<TUPLETYPE>.  Only std::string is proven to work so far
 * @tparam MATCHSCHEME indicating the string matching scheme applied in PEAT.  Only LinearStrMatch is proven to work so far
 * @tparam ADAPTER_COMPARE_SCHEME indicating the adapter comparing scheme that is going to apply in adapter_check function.  It can be either double or int.
 * @tparam GENE_COMPARE_SCHEME indicating the adapter comparing scheme that is going to apply in gene_check function.  It can be either double or int.
 */
template <  typename DATATYPE, 
			typename MATCHSCHEME, 
			typename ADAPTER_COMPARE_SCHEME, 
			typename GENE_COMPARE_SCHEME 
		 >
class TrimTrait
{
public:
	typedef DATATYPE DataType;
	typedef MATCHSCHEME MatchScheme;
	typedef typename MatchScheme::mismatch_indicator_type MismatchIndicatorType;
	typedef ADAPTER_COMPARE_SCHEME adapter_compare_scheme_type;
	typedef GENE_COMPARE_SCHEME gene_compare_scheme_type;
};

/**
 * @class ParameterTrait
 * @brief provide a trait to give parameters needed for PEAT
 * @tparam TRAIT must be a derived type of templated TrimTrait class, where four types of DATATYPE, MATCHSCHEME, ADAPTER_COMPARE_SCHEME, GENE_COMPARE_SCHEME will be determined.  TRAIT is defaulted to be TrimTrait <std::string, LinearStrMatch<double>, double, double>
 */
template < typename TRAIT = TrimTrait <std::string, LinearStrMatch<double>, double, double> >
struct ParameterTrait 
{
	size_t startsize;// indicating the length of reversed-complement comparsion fragment
	size_t num, pool_size; //respectively indicate how many pieces of data, in type of FORMAT<TUPLETYPE>, are processed in a single round of trimming operation and
						   //how many threads are going to be used
	typename TRAIT::MismatchIndicatorType m_indicator; //indicating max mismatch tolerance in rc compare opeartion, double for ratio and size_t for length. 
	typename TRAIT::adapter_compare_scheme_type a_mismatch; //indicating max mismatch tolerance in adapter compare operation
	typename TRAIT::gene_compare_scheme_type g_mismatch; //indicating max mismatch tolerance in gene compare operation

/**
 * @brief constructor 
 */
	ParameterTrait (  size_t startsize_in=30
					, typename TRAIT::MismatchIndicatorType rc_ind=0.3
					, typename TRAIT::adapter_compare_scheme_type a_ind=0.6
					, typename TRAIT::gene_compare_scheme_type g_ind=0.4
					, size_t num_in=100000
					, size_t pool_size_in=1
					)
		: startsize (startsize_in)
		, num (num_in)
		, pool_size (pool_size_in)
		, m_indicator (rc_ind)
		, a_mismatch (a_ind)
		, g_mismatch (g_ind)
	{}
};

/**
 * @class PairEndAdapterTrimmer_impl
 * @brief provide implementation detail for PEAT
 * @tparam template template parameter FORMAT, indicating what type of data PEAT is going to take
 * @tparam TUPLETYPE indicating the exact format that keep track of the FORMAT data 
 * @tparam TRAIT must be a derived type of templated TrimTrait class, where four types of DATATYPE, MATCHSCHEME, ADAPTER_COMPARE_SCHEME, GENE_COMPARE_SCHEME will be determined.  TRAIT is defaulted to be TrimTrait <std::string, LinearStrMatch<double>, double, double>
 */
template < 
		template <typename> class FORMAT,
		typename TUPLETYPE,
		typename TRAIT
		 >
class PairEndAdapterTrimmer_impl
{ 
//public:
//	std::shared_ptr < std::map < int, std::vector<size_t> > > trim_position;
protected:
/**
 * @memberof PairEndAdapterTrimmer_impl
 * @brief a member element keeping the complement mapping table
 */
	mutable std::unordered_map < char, char > complement_map;
/**
 * @memberof PairEndAdapterTrimmer_impl
 * @brief a member element keeping the parameter_trait object
 */
	ParameterTrait <TRAIT> parameter_trait;
/**
 * @memberof PairEndAdapterTrimmer_impl
 * @brief a member element keeping mismatch string matching parameter for handling 1st level string matching operation. It can be a double, representing the maximum tolerable mismatch ratio, or a size_t/int, representing the maximum tolerable mismatch character count
 */
	typename TRAIT::MismatchIndicatorType mismatch_indicator;
/**
 * @memberof PairEndAdapterTrimmer_impl
 * @brief a member element keeping mismatch string matching parameter for handling adapter comparsion of 2nd level string matching operation. A same double/size_t behavior is also adopted here
 */
	typename TRAIT::adapter_compare_scheme_type mismatch_indicator_adapter;
/**
 * @memberof PairEndAdapterTrimmer_impl
 * @brief a member element keeping mismatch string matching parameter for handling gene comparsion of 2nd level string matching operation. A same double/size_t behavior is also adopted here
 */
	typename TRAIT::gene_compare_scheme_type mismatch_indicator_gene;

public:
/**
 * @brief constructor
 */
	PairEndAdapterTrimmer_impl (//std::map<int, std::vector< FORMAT<TUPLETYPE> > >* content_ptr,  
								  ParameterTrait <TRAIT> i_parameter_trait )
//		: trim_position ( std::make_shared<std::map < int, std::vector<size_t> > > () )
		: complement_map ({ {'A','T'}, {'C','G'}, {'G','C'}, {'T','A'}, {'N','N'} }) 
		, parameter_trait (i_parameter_trait)
		, mismatch_indicator ( i_parameter_trait.m_indicator )
		, mismatch_indicator_adapter (i_parameter_trait.a_mismatch)
		, mismatch_indicator_gene (i_parameter_trait.g_mismatch)
	{}

/**
 * @brief main interface for conducting trimming operation, where 1st and 2nd levels of string matching operations will be conducted.
 */
	void TrimImpl (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, std::vector<size_t>& trim_pos, size_t start, size_t dif)
	{
		trim_pos.clear();
		trim_pos.reserve (dif);
		for ( size_t itr=start; itr!=start+dif; ++itr)
		{
			int current_trimpos = -1;
			size_t mask = parameter_trait.startsize;//10
			// 1st order string matching, get short gene start paragraph from 1st read, have it reverse complemented, and compare with 2nd read; vice versa 
			std::vector<size_t> trim_impl_result1, trim_impl_result2;
			RC_Compare (result2, trim_impl_result1, trim_impl_result2, itr, mask);
			//scanning through the intersection of trim_impl_result1 and 2
			std::vector<size_t> trim_result (trim_impl_result1.size()+trim_impl_result2.size());
			auto itr_end = std::set_intersection ( trim_impl_result1.begin(), trim_impl_result1.end(), 
				trim_impl_result2.begin(), trim_impl_result2.end(), trim_result.begin() );
			trim_result.resize (itr_end - trim_result.begin());

			if ( trim_result.size() == 0)
				trim_pos.push_back (0); 
			else if ( trim_result.size() == 1)
			{//directly accept current_trimpos when only one trim_result is presented in the trim_result
				current_trimpos = trim_result.back();
				trim_pos.push_back (current_trimpos);
				std::get<1>( ((*result2)[0][itr]).data ).resize (current_trimpos);
				std::get<3>( ((*result2)[0][itr]).data ).resize (current_trimpos);
				std::get<1>( ((*result2)[1][itr]).data ).resize (current_trimpos);
				std::get<3>( ((*result2)[1][itr]).data ).resize (current_trimpos);
			}
			else
			{//scaning through each of the trim_result to find an uptimum trim position
				int match_index_gene = -1;//match_index_adapter = -1, 
				while (trim_result.size()!=0)
				{
					int match_index_gene_temp = -1;
					size_t current_trimpos_temp = trim_result.back();
					// 2nd and 3rd orders of string matching, ckeck the identity of the obtained adapters and the reverse_complementability of the obtained genes
					if ( AdapterCheck (result2, itr, current_trimpos_temp) && 
						 GeneCheck (result2, itr, current_trimpos_temp, match_index_gene_temp) &&
						 match_index_gene_temp > match_index_gene )
					{
						match_index_gene = match_index_gene_temp;
						current_trimpos = current_trimpos_temp;
					}
					trim_result.pop_back();
				}
				if (current_trimpos==-1)
					trim_pos.push_back (0);	//keep trim_pos having same sizes as th-start
				else
				{
					trim_pos.push_back (current_trimpos);
					std::get<1>( ((*result2)[0][itr]).data ).resize (current_trimpos);
					std::get<3>( ((*result2)[0][itr]).data ).resize (current_trimpos);
					std::get<1>( ((*result2)[1][itr]).data ).resize (current_trimpos);
					std::get<3>( ((*result2)[1][itr]).data ).resize (current_trimpos);
				}
			}
		}
	}		

protected:	
/**
 * @brief a function capable of having paired reads in result2 adjusted with same length
 */
	void check_read_length (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2)
	{
		if ( std::get<1>( (*result2)[0][0].data ).size() != std::get<1>( (*result2)[1][0].data ).size() )
		{
			size_t num = std::min ( (*result2)[0].size(), (*result2)[1].size() );
			for ( auto index=0; index!= num; ++index )
			{
				size_t length = std::min ( std::get<1>( (*result2)[0][index].data ).size(), std::get<1>( (*result2)[1][index].data ).size() );
				std::get<1>((*result2)[0][index].data).resize ( length );
				std::get<3>((*result2)[0][index].data).resize ( length );
				std::get<1>((*result2)[1][index].data).resize ( length );
				std::get<3>((*result2)[1][index].data).resize ( length );
			}
			(*result2)[0].resize (num);
			(*result2)[1].resize (num);
		}
	}

/**
 * @brief implemenation detail to carry out 1st level string search, where we have a front portion of one of the paired sequences reversed and complemented, and compared to the other paired sequences, and the other way around
 */
	void RC_Compare ( std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, std::vector<size_t>& trim_impl_result1, std::vector<size_t>& trim_impl_result2, size_t itr, size_t mask ) 
	{
		boost::iterator_range <std::string::iterator> rcstr_1 
			( std::get<1>( ((*result2)[0][itr]).data ).begin(), std::get<1>( ((*result2)[0][itr]).data ).begin()+mask );
		boost::iterator_range <std::string::iterator> read_2 
			( std::get<1>( (*result2)[1][itr].data ).begin(),	std::get<1>( (*result2)[1][itr].data ).end() );
		boost::iterator_range <std::string::iterator> rcstr_2 
			( std::get<1>( ((*result2)[1][itr]).data ).begin(), std::get<1>( ((*result2)[1][itr]).data ).begin()+mask );
		boost::iterator_range <std::string::iterator> read_1 
			( std::get<1>( (*result2)[0][itr].data ).begin(),	std::get<1>( (*result2)[0][itr].data ).end() );

		RC_Compare_function_impl(trim_impl_result1, rcstr_1, read_2);
		RC_Compare_function_impl(trim_impl_result2, rcstr_2, read_1);
		if ( !trim_impl_result1.empty() && trim_impl_result1.back() == std::get<1>( ((*result2)[0][itr]).data ).size())
			trim_impl_result1.pop_back();
		if ( !trim_impl_result2.empty() && trim_impl_result2.back() == std::get<1>( ((*result2)[1][itr]).data ).size())
			trim_impl_result2.pop_back();
	}

/**
 * @brief implemenation detail for 1st order level string search. Template parameter T is used to take either string or boost::iterator_range object when processing the string match operation.  
 * @tparam T supporting types of std::string and boost::iterator_range so far
 */
//1st string search impl, function version * iterator_range impl 
	template <typename T>
	void RC_Compare_function_impl (std::vector<size_t>& found_vec, T& adapter, T& txt)	//adapter is the reverse_complement pattern
	{
		std::string rc_adapter_str (adapter.size(), '1');
		T rc_adapter (rc_adapter_str.begin(), rc_adapter_str.end());
		GetReverseComplement (adapter, rc_adapter);
		size_t txt_length = txt.size(), adapter_length = adapter.size();
		size_t i=0,  jj=adapter_length-1, msize;

		if ( IsInDoubleType <typename TRAIT::MismatchIndicatorType>::value )
			msize = mismatch_indicator* adapter_length;	//path for mismatch_indicator is in type of double
		else
			msize = mismatch_indicator; //path for mismatch indicator is in size_t or int

		while ( i<=txt_length-adapter_length )
		{
			size_t mmc = msize;
			int j=jj;
			while ( j>=0 && (rc_adapter[j]==txt[i+j] || mmc !=0) )
			{
				if (rc_adapter[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				found_vec.push_back (i+adapter_length);
				++i;
			}
			else
				++i;
		}
	}

/**
 * @brief implemenation detail for adapter check in 2nd order level string search, where identifed adapter portions are checked whether they are substantially the same.  
 */
//2nd string serach impl, functor version
	bool AdapterCheck (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, size_t itr, size_t current_trimpos)
	{
//functor * string version	avg 8.21sec when 1nd & 3rd both in function * iterator version  
		std::string adapter1 ( std::get<1> (((*result2)[0][itr]).data).substr (current_trimpos) );
		std::string adapter2 ( std::get<1> (((*result2)[1][itr]).data).substr (current_trimpos) );

		if ( IsInDoubleType <typename TRAIT::adapter_compare_scheme_type>::value )
		{
			int break_condition = mismatch_indicator_adapter* adapter1.size();
			for ( size_t i = 0; i!=adapter1.size(); ++i )
			{
				if (adapter1[i]!=adapter2[i])
					--break_condition;
			}
			if (break_condition < 0)
				return false;
			else
				return true;
		}
		else
		{
			int break_condition = mismatch_indicator_adapter;
			for ( size_t i = 0; i!=adapter1.size(); ++i )
			{
				if (adapter1[i]!=adapter2[i])
					--break_condition;
			}
			if (break_condition < 0)
				return false;
			else
				return true;
		}
	}

/**
 * @brief implemenation detail for gene check in 2nd order level string search, where identifed gene portions are checked whether they are mutually reversed and complemented 
 */
//3nd string serach impl, functor version
	bool GeneCheck (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, size_t itr, size_t current_trimpos, int& match_index_gene_temp)
	{
//functor * iterator range avg 8.31sec when 1st in function * iterator version, 2nd in function * string version  
		boost::iterator_range <std::string::iterator> gene1 
			( std::get<1> (((*result2)[0][itr]).data).begin(), //substr (current_trimpos);
				std::get<1> (((*result2)[0][itr]).data).begin()+current_trimpos );
		std::string str_rc (gene1.size(), '1');
		boost::iterator_range <std::string::iterator> gene1_rc (str_rc.begin(), str_rc.end()) ;
		GetReverseComplement (gene1, gene1_rc);  
		boost::iterator_range <std::string::iterator> gene2 
			( std::get<1> (((*result2)[1][itr]).data).begin(), //substr (current_trimpos);  
				std::get<1> (((*result2)[1][itr]).data).begin()+current_trimpos );

		if ( IsInDoubleType <typename TRAIT::gene_compare_scheme_type>::value )
		{
			int break_condition = mismatch_indicator_gene* gene1_rc.size();
			for ( auto i = 0; i!=gene1_rc.size(); ++i )
			{
				if (gene1_rc[i]!=gene2[i])
					--break_condition;
			}
			if (break_condition < 0)
				return false;
			else
			{
				match_index_gene_temp = break_condition;
				return true;
			}
		}
		else
		{
			int break_condition = mismatch_indicator_gene;
			for ( auto i = 0; i!=gene1_rc.size(); ++i )
			{
				if (gene1_rc[i]!=gene2[i])
					--break_condition;
			}
			if (break_condition < 0)
				return false;
			else
			{
				match_index_gene_temp = break_condition;
				return true;
			}
		}
	}

/**
 * @brief implemenation detail of reversed and complement operation in boost::iterator_range version
 */
	//boost::iterator_range version 
	void GetReverseComplement (boost::iterator_range <std::string::iterator>& Strin, boost::iterator_range <std::string::iterator>& StrResult)
	{
		size_t index = 0;
		std::for_each (Strin.begin(), Strin.end(),
			[&] (char& Q)
			{	
				StrResult[index] = complement_map [Q];
				++index;
			});
		RangeIteratorReverse<std::string::iterator> (StrResult.begin(), StrResult.end());
	}

/**	
 * @brief implemenation detail of reversed and complement operation in std::string version
 */
	//std::string version
	void GetReverseComplement (std::string& Strin, std::string& StrResult)
	{
		StrResult.clear();
		size_t index = 0;
		std::for_each (Strin.begin(), Strin.end(),
			[&] (char& Q)
			{	StrResult.push_back ( complement_map[Q] ) ; });
		std::reverse (StrResult.begin(), StrResult.end());
	}

	inline void VerboseImpl ( bool flag, uint32_t& read_count, int& flag_type )
	{
		if ( flag == true )
		{
			if ( flag_type == 0 )
			{	
				std::cerr << "=========================================================\n";
				std::cerr << "=====================Program process=====================\n";
				std::cerr << "The number of reads that are already trimmed:\n";
				++flag_type;
			}
			if ( flag_type == 2 )
			{
				std::cerr << "\n================The program is over.=====================\n";
			}
			else if ( flag_type == 1 )
				std::cerr << read_count << "\r";
			else
				;	
		}
	}

	inline void SumImpl ( std::vector< FORMAT<TUPLETYPE> >& result, uint32_t& sum_read_lengths, uint32_t& sum_read_counts )
	{
		for ( auto& fq: result )
		{
			sum_read_lengths+= std::get<1>( fq.data ).size();
			++sum_read_counts;
		}
	}	

};
#endif
