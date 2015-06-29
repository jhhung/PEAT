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
	{
		if ( traitin.threads_ != 0)
			ptr_to_GlobalPool_peat_ -> ChangePoolSize (traitin.threads_);
	}

	inline void QTrim (std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result2, int map_index=0 )
	{
		Job_distributer_pipeline <ParallelTypes::M_T, std::vector<FORMAT<TUPLETYPE> >, std::vector<int> > jd;
		jd.distribute_Q_jobs( (*result2)[map_index], 
		[this] (FORMAT<TUPLETYPE>& format_data)
		{
			this->QTrimImpl (format_data);
		}
		);
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

	void Verbose ( bool flag, uint32_t count_reads, int& flag_type )
	{
		this->VerboseImpl ( flag, count_reads, flag_type );
	}   
	
	void Sum ( std::map < int, std::vector< FORMAT<TUPLETYPE> > >* result, uint32_t& sum_read_lengths, uint32_t& sum_read_counts, int map_index=0 )
	{
		this-> SumImpl( (*result)[ map_index ], sum_read_lengths, sum_read_counts );
	}

	void Summary ( uint32_t sum_length, uint32_t sum_reads, std::string adapterSeq, int value, std::ostream* out_report )
	{                                                                                                              
        double average_length ( double(sum_length)/double(sum_reads) );
		if ( value == 0 )
		{	
			(*out_report) << "PEAT report\nMode:\tsingle-end";
			(*out_report) << "\nTotal number of reads:\t" << sum_reads;
			(*out_report) << "\nAverage length of reads in raw data:\t" << average_length;
			out_report = NULL;
		}
		else if ( value == 1 )
		{
			(*out_report) << "\nAverage length of reads after quality trimming:\t" << average_length;
			out_report = NULL;
		}
		else if ( value == 2 )
		{
			(*out_report) << "\nAverage length of reads after adapter trimming:\t" << average_length;
			(*out_report) << "\nAdapter sequence (user applied):\t" << adapterSeq << "\n";
			out_report = NULL;
		}
	}                                                                                                              

};

#endif
