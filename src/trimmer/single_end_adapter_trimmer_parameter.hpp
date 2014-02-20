#ifndef SINGLE_END_ADAPTER_TRIMMER_HPP_
#define SINGLE_END_ADAPTER_TRIMMER_HPP_
#include "single_end_adapter_trimmer_impl_parameter.hpp"
#include "../constant_def.hpp"
#include "../aligner/job_distributer.hpp"

template < 
		ParallelTypes ParaType, 
		template <typename> class FORMAT,
		typename TUPLETYPE
		 >
class SingleEndAdapterTrimmer 
{};
 
template < 
		template <typename> class FORMAT,
		typename TUPLETYPE
		 >
class SingleEndAdapterTrimmer < ParallelTypes::M_T, FORMAT, TUPLETYPE>
	: public SingleEndAdapterTrimmer_impl < FORMAT, TUPLETYPE >
{ 
protected:
	ThreadPool* ptr_to_GlobalPool_peat_;

public:
	typedef FORMAT<TUPLETYPE> format_type;
	typedef TUPLETYPE tuple_type;
	std::shared_ptr < std::map < int, std::vector<size_t> > > trim_position;
	SingleEndAdapterTrimmer ( ParameterTraitSeat traitin, ThreadPool* tp = &GlobalPool )
		: SingleEndAdapterTrimmer_impl < FORMAT, TUPLETYPE > ( traitin )
		, ptr_to_GlobalPool_peat_ (tp)
		, trim_position ( std::make_shared<std::map < int, std::vector<size_t> > > () ) 
	{}

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
