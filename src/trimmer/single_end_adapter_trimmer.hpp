/**
 *  @file single_end_adapter_trimmer.hpp
 *  @brief provide SEAT implementation detail
 *  @author C-Salt Corp.
 */
#ifndef SINGLE_END_ADAPTER_TRIMMER_HPP_
#define SINGLE_END_ADAPTER_TRIMMER_HPP_
#include "single_end_adapter_trimmer_impl.hpp"
#include "../constant_def.hpp"
#include "../aligner/job_distributer.hpp"
/**
 * @class SingleEndAdapterTrimmer
 * @brief provide a generic form of SEAT
 * @tparam none-type parameter of int / enum of ParallelTypes, indicating what policies that PEAT is going to employ
 * @tparam template template parameter of FORMAT, designed to be any one of current data formats
 * @tparam TUPLETYPE indicating data format of the got piece of data
 * @tparam TRAIT indicating the TrimTrait format that is going to apply
 */
template < 
		ParallelTypes ParaType, 
		template <typename> class FORMAT,
		typename TUPLETYPE,
		QualityScoreType QSType
		 >
class SingleEndAdapterTrimmer 
{};
 
/**
 * @class SingleEndAdapterTrimmer
 * @brief specialized form of the SingleEndAdapterTrimmer, with the ParallelType specialized to be ParallelTypes::NORMAL, indicating non-parallel scheme
 */
template < 
		template <typename> class FORMAT,
		typename TUPLETYPE,
		QualityScoreType QSType
		 >
class SingleEndAdapterTrimmer < ParallelTypes::NORMAL, FORMAT, TUPLETYPE, QSType >
	: public SingleEndAdapterTrimmer_impl < FORMAT, TUPLETYPE, QSType >
{ 
public:
/**
 * @typedef format_type
 * @brief define a trait access for the type of FORMAT<TUPLETYPE>
 */
	typedef FORMAT<TUPLETYPE> format_type;
/**
 * @typedef tuple_type
 * @brief define a trait access for the type of TUPLETYPE
 */
	typedef TUPLETYPE tuple_type;

//	leave for now, may be removed
//	typedef TRAIT peat_trait_type;
//	typedef ParameterTraitSeat <TRAIT> parameter_trait_type;
//

/**
 * @brief define a data structure keeping track of the trimming position obtained in Trim funciton 
 */
	std::shared_ptr < std::map < int, std::vector<size_t> > > trim_position;
/**
 * @brief constructor
 */
	SingleEndAdapterTrimmer ( ParameterTraitSeat <QSType> traitin )
		: SingleEndAdapterTrimmer_impl < FORMAT, TUPLETYPE, QSType > ( traitin )
		, trim_position ( std::make_shared<std::map < int, std::vector<size_t> > > () ) 
	{}

/**
 * @brief main interface for conducting trimming operation.  
 */
	inline void QTrim ( std::map < int, std::vector< FORMAT< TUPLETYPE > > >* result2, std::vector<int>& tirm_pos )
	{

	}
    inline void Trim (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, std::vector<int>& trim_pos)
    {
        trim_pos.clear();
        trim_pos.reserve ((*result2)[0].size());//(dif);
        for (size_t itr=0; itr!=(*result2)[0].size(); ++itr)//( size_t itr=start; itr!=start+dif; ++itr)
        {
std::cerr<<"processing fastq# "<<itr<<'\r';
			this->TrimImpl ((*result2)[0][itr], trim_pos);
        }
    }
};

/**
 * @class SingleEndAdapterTrimmer
 * @brief specialized form of the SingleEndAdapterTrimmer, with the ParallelType specialized to be ParallelTypes::NORMAL, indicating multi-threading parallel scheme
 */
template < 
		template <typename> class FORMAT,
		typename TUPLETYPE,
		QualityScoreType QSType
		 >
class SingleEndAdapterTrimmer < ParallelTypes::M_T, FORMAT, TUPLETYPE, QSType >
	: public SingleEndAdapterTrimmer_impl < FORMAT, TUPLETYPE, QSType >
{ 
protected:
/**
 * @memberof SingleEndAdapterTrimmer
 * @brief define a normal pointer pointing to a ThreadPool object
 */
	ThreadPool* ptr_to_GlobalPool_peat_;

public:
/**
 * @typedef format_type
 * @brief define a trait access for the type of FORMAT<TUPLETYPE>
 */
	typedef FORMAT<TUPLETYPE> format_type;
/**
 * @typedef tuple_type
 * @brief define a trait access for the type of TUPLETYPE
 */
	typedef TUPLETYPE tuple_type;

//
//	typedef TRAIT peat_trait_type;
//	typedef ParameterTraitSeat <TRAIT> parameter_trait_type;

/**
 * @memberof SingleEndAdapterTrimmer
 * @brief define a data structure keeping track of the trimming position obtained in Trim funciton 
 */
	std::shared_ptr < std::map < int, std::vector<size_t> > > trim_position;
/**
 * @brief constructor
 */
	SingleEndAdapterTrimmer ( ParameterTraitSeat <QSType> traitin, ThreadPool* tp = &GlobalPool )
		: SingleEndAdapterTrimmer_impl < FORMAT, TUPLETYPE, QSType > ( traitin )
		, ptr_to_GlobalPool_peat_ (tp)
		, trim_position ( std::make_shared<std::map < int, std::vector<size_t> > > () ) 
	{}

/**
 * @brief main interface for conducting trimming operation.  
 */
/*
	inline void Trim (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, int nthreads, std::vector<int>& trim_pos)
	{
		trim_pos.clear();
		trim_pos.reserve ((*result2)[0].size());

		Job_distributer_pipeline <ParallelTypes::M_T, std::vector<FORMAT<TUPLETYPE> >, std::vector<int> > jd;
		jd.distribute_jobs( (*result2)[0], trim_pos, nthreads,
		[this] (FORMAT<TUPLETYPE>& format_data, std::vector<int>& out_buffer)
		{
			this->TrimImpl (format_data, out_buffer);
		}
		);
	}
*/
	inline void QTrim ( std::map < int, std::vector< FORMAT< TUPLETYPE > > >* result2, int nthreads, std::vector<int>& tirm_pos, int map_index=0 )
	{

	}

	inline void Trim (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, int nthreads, std::vector<int>& trim_pos, int map_index=0)
	{
		trim_pos.clear();
		trim_pos.reserve ((*result2)[map_index].size());

		Job_distributer_pipeline <ParallelTypes::M_T, std::vector<FORMAT<TUPLETYPE> >, std::vector<int> > jd;
		jd.distribute_jobs( (*result2)[map_index], trim_pos, nthreads,
		[this] (FORMAT<TUPLETYPE>& format_data, std::vector<int>& out_buffer)
		{
			this->TrimImpl (format_data, out_buffer);
		}
		);
	}
};

#endif
