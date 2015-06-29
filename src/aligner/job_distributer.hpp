#ifndef JOB_DISTRIBUTER_HPP_
#define JOB_DISTRIBUTER_HPP_

#include <vector>
#include <fstream>
#include <iostream>

#include "../constant_def.hpp"
//#include "../thread_pool_update.hpp"
#include "../thread_control_version.hpp"
//#include "aligner_trait.hpp"
//#include "genome_pre_handler.hpp"
//#include "aligner_table.hpp"
//#include "searcher.hpp"
//#include "indexer.hpp"

template<ParallelTypes ParallelType, class InType, class OutType>
class Job_distributer_pipeline
{

};

template<class InType, class OutType>
class Job_distributer_pipeline <ParallelTypes::M_T, InType, OutType>
{
private:
	std::mutex reader_mutex, writer_mutex;
	INTTYPE in_group_reads_number;
public:
	Job_distributer_pipeline()
		:in_group_reads_number(100)
	{};
	
	void distribute_jobs(
			InType &datas
		, OutType &out
		, std::size_t nthreads
		, std::function<void(typename InType::value_type &, OutType &)> functor
	)
	{
//		std::cout << "AA" << std::endl;
//		GlobalPool.ChangePoolSize(nthreads);
		std::vector<OutType*> tmp_out( std::ceil(datas.size()/(double)in_group_reads_number));
		INTTYPE group_idx (0);
		//std::cout << "B" << std::endl;
		std::vector<size_t> jobs;
	
//		std::cerr << "out.size():" << out.size() << "\n";
		
		for(auto i( datas.begin() ); i < datas.end(); i += in_group_reads_number)
		{
			auto range_begin (i);
			auto range_end (i + in_group_reads_number);
			//std::cout << "begin " << range_begin-datas.begin() << " end " << range_end-datas.begin() << std::endl;
			if(range_end > datas.end())
				range_end = datas.end();

			auto job_id = GlobalPool.JobPost(
				[this, range_begin, range_end, group_idx, &tmp_out, &functor]()
				{
					(tmp_out[group_idx]) = new OutType();
					std::for_each(range_begin, range_end, 
						[&tmp_out, &functor, group_idx](typename InType::value_type & data)
						{
							functor(data, *(tmp_out[group_idx]) );
						}
					);
				}, std::vector<size_t>(0) 
			);
			++group_idx;
			jobs.push_back(job_id);
		}

		for( auto i : jobs)
		{
			GlobalPool.FlushOne(i);
		}
		//GlobalPool.ResetPool();
		
		
		
		for(auto &tmp_out_child : tmp_out)
		{
			INTTYPE tmp = tmp_out_child->size();
			std::move( tmp_out_child->begin(), tmp_out_child->end(), std::back_inserter(out) );
			delete tmp_out_child;
		}
	}
	
	void distribute_Q_jobs(
			InType &datas
		, std::function<void(typename InType::value_type &)> functor
	)
	{
	//	std::vector<OutType*> tmp_out( std::ceil(datas.size()/(double)in_group_reads_number));
		INTTYPE group_idx (0);
		std::vector<size_t> jobs;
		
		for(auto i( datas.begin() ); i < datas.end(); i += in_group_reads_number)
		{
			auto range_begin (i);
			auto range_end (i + in_group_reads_number);
			//std::cout << "begin " << range_begin-datas.begin() << " end " << range_end-datas.begin() << std::endl;
			if(range_end > datas.end())
				range_end = datas.end();

			auto job_id = GlobalPool.JobPost(
//				[this, range_begin, range_end, group_idx, &tmp_out, &functor]()
				[this, range_begin, range_end, group_idx, &functor]()
				{
	//				(tmp_out[group_idx]) = new OutType();
					std::for_each(range_begin, range_end, 
//						[&tmp_out, &functor, group_idx](typename InType::value_type & data)
						[&functor](typename InType::value_type & data)
						{
//							functor(data, *(tmp_out[group_idx]) );
							functor(data);
						}
					);
				}, std::vector<size_t>(0) 
			);
			++group_idx;
			jobs.push_back(job_id);
		}
		for( auto i : jobs)
		{
			GlobalPool.FlushOne(i);
		}
		//GlobalPool.ResetPool();
/*		
		for(auto &tmp_out_child : tmp_out)
		{
			INTTYPE tmp = tmp_out_child->size();
			std::move( tmp_out_child->begin(), tmp_out_child->end(), std::back_inserter(out) );
			delete tmp_out_child;
		}
*/
	}

};



template<ParallelTypes ParallelType, class QueryParserType>
class Job_distributer
{};

template<class QueryParserType>
class Job_distributer<ParallelTypes::NORMAL, QueryParserType>
{
public:
	Job_distributer();
};

class VectorParser
{};




template<class QueryParserType>
class Job_distributer<ParallelTypes::M_T, QueryParserType>
{
private:
	std::mutex reader_mutex, writer_mutex;
	INTTYPE in_group_reads_number;
public:
	Job_distributer()
		:in_group_reads_number(1000)

	{};
	
	
	void distribute_jobs(
			QueryParserType &file_parser
		, std::ofstream &out
		, std::size_t nthreads
		, std::function<void(typename QueryParserType::format_type &, std::stringstream &)> functor
		)
	{
//		GlobalPool.ChangePoolSize(nthreads);

		bool eof_flag(false);
		int file_idx(0);
		while( !eof_flag )
		{
			std::vector< typename QueryParserType::format_type > *group = new std::vector< typename QueryParserType::format_type >();
			group->reserve( in_group_reads_number );
			
			for(INTTYPE i(0); i<in_group_reads_number; ++i)
			{
				group->emplace_back( std::move (file_parser.get_next_entry (file_idx) ) );
				if ( group->back().eof_flag )
				{
					//remove last nothing fastq
					group->pop_back();
					++file_idx;
					if(file_parser.file_num_ == file_idx)
					{
						eof_flag = true;
						break;
					}
				}
			}
			if(group->size() == 0)
				break;
			GlobalPool.JobPost(
				[this, group, &out, &functor]()
				{
					decltype(group) GROUP(group);
					std::stringstream out_buffer;
					for ( auto &fp_v : *GROUP)
					{
						functor(fp_v, out_buffer);
					}
					{
						std::lock_guard<std::mutex> lock(this->writer_mutex);
						out << out_buffer.rdbuf();
						out_buffer.str("");
					}
					delete GROUP;
				}, std::vector<size_t>(0) 
			);
		}
		GlobalPool.FlushPool();
		
	}
	
	void distribute_jobs2(
			QueryParserType &file_parser
		, std::ofstream &out
		, std::size_t nthreads
		, std::function<void(typename QueryParserType::format_type &, std::stringstream &)> functor
		)
	{
		size_t file_idx(0);
		bool eof_flag(false);
//		GlobalPool.ChangePoolSize(nthreads);
		
		for(auto i(0); i<nthreads; ++i )
		{
			
			GlobalPool.JobPost( 
				[this, &file_parser, &file_idx, &eof_flag, &out, &functor]()
				{
					std::stringstream out_buffer;
					
					std::vector< typename QueryParserType::format_type > group;
					group.reserve( this->in_group_reads_number );
					while(true)
					{
						{ // ! lock_guard 解構
							std::lock_guard<std::mutex> lock(reader_mutex);
							if(eof_flag)
								break;
								
							// ! read fastq reads to a group
							uint64_t num (0);
							for(uint64_t num(0); num != this->in_group_reads_number; ++num)
							{
								// ! read one fastq read and save into group
								group.emplace_back( std::move (file_parser.get_next_entry (file_idx) ) );
								//std::cerr << group.back().eof_flag << std::endl;
								if ( group.back().eof_flag )
								{
									
									//remove last nothing fastq
									group.pop_back();	
									// this file is eof, change to next file
									file_idx++;
									if(file_parser.file_num_ == file_idx)
										eof_flag = true;
									break;
								}
								++num;
							}
							
							// ! 此檔案已經讀完，讀下一個檔案
							if(group.size() == 0)
								continue;
						} // ! lock_guard 解構
						// ! for searching...
						for ( auto &fp_v : group)
						{
							functor(fp_v, out_buffer);
						}
						// ! for writing into file...
						{
							std::lock_guard<std::mutex> lock(writer_mutex);
							out << out_buffer.rdbuf();
							out_buffer.str("");
						}
						
						group.clear();
					}
				}
				, std::vector<size_t>(0) 
			);
			
			
		}
		GlobalPool.FlushPool();
	}
	
	
};
#endif
