///@file thread_pool.hpp
///@brief Define a thread pool, and accordingly provide a GLOBAL thread pool object, designed to be a default multi-thread operation thread pool 
///@author C-Salt Corp.
#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_
#include <boost/ref.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <future>
#include <utility>
#include <chrono>

struct function_wrapper
{
	template < typename FUNC >
	void operator() ( FUNC&& func, std::shared_ptr < std::promise <size_t> > sptr, size_t& run_job_count, std::mutex& mut )
	{
		func ();
		{
			std::lock_guard<std::mutex> lock (mut);
			++run_job_count;
		}
		sptr -> set_value (0);
	}
};

struct brutal_function_wrapper
{
	template < typename FUNC >
	void operator() ( FUNC&& func, std::shared_ptr < std::promise <size_t> > sptr )
	{
		func ();
		sptr -> set_value (0);
	}
};

class ThreadPool
{
private:
	boost::asio::io_service io_service_;
	boost::thread_group threads_;
	boost::shared_ptr < boost::asio::io_service::work > work_; 
	std::mutex my_mute2;
	std::vector < std::future<size_t> > future_map_;
//	std::mutex new_mut;
//	std::mutex new_mut2;

public:
	size_t run_job_count_;
	std::mutex mute_;
	size_t pool_size_;
	size_t unrun_ratio_;

	ThreadPool ( size_t pool_size, size_t unrun_ratio_bound=10 )
		: work_ ( new boost::asio::io_service::work ( io_service_ ) )
		, pool_size_ ( pool_size )
		, run_job_count_ (0)
		, unrun_ratio_ (unrun_ratio_bound)
	{
		for ( size_t i = 0; i < pool_size_; ++i )
			threads_.create_thread ( boost::bind ( &boost::asio::io_service::run, &io_service_ ) );
	}

	void ChangePoolSize ( std::size_t new_pool_size)
	{
		pool_size_ = new_pool_size;
		ResetPool();//FlushPool(0);
		//future_map_.clear();
		//run_job_count_=0;
	}

	void ChangeUnrunRatio ( std::size_t new_ratio)
	{
		unrun_ratio_ = new_ratio;
		ResetPool();//FlushPool(0);
		//future_map_.clear();
		//run_job_count_=0;
	}

	void ResetPool (void)//FlushPool (size_t i)
	{
		FlushPool ();
		work_.reset();			
		threads_.join_all();	// have all threads terminated to gather the corresponding results
		io_service_.reset();	// reset io_service_ object for future invocation of boost::asio::io_service::run(), run_one(), poll(), or poll_one()
		work_.reset( new boost::asio::io_service::work( io_service_ ) );
		for ( std::size_t i = 0; i < pool_size_; ++i )
			threads_.create_thread ( boost::bind ( &boost::asio::io_service::run, &io_service_ ) );
		run_job_count_=0;
		future_map_.clear();
		brutal_future_map_.clear();
	}

	~ThreadPool ()
	{
		work_.reset();
		threads_.join_all();
	}

	template <typename JOB>
	void PostImpl ( JOB&& job, std::shared_ptr < std::promise <size_t> >& sptr )
	{
		function_wrapper func_wrap;
		io_service_.post ( std::bind (func_wrap, std::forward<JOB&&>(job), sptr, std::ref(run_job_count_), std::ref(mute_) ) );
	}

	bool CheckDependency (const std::vector<size_t>& dependency_table)
	{
		size_t dependency_met = 0;
		for ( auto& ii : dependency_table )
		{
			if ( future_map_[ii].valid() )	
			{
				if ( future_map_[ii].wait_for (std::chrono::microseconds(100)) == std::future_status::ready ) 
					++dependency_met;
				else
					;
			}
			else
				++dependency_met;
		}
		return ( dependency_met == dependency_table.size() );
	}

	template < typename JOB >
	int JobPost ( JOB&& job, const std::vector<size_t>& dependency_table = std::vector<size_t>(0) ) 
	//&& here is actually an "universal reference" acording to Scott Meyers, and it can be deducted as both r and l refrence formats.
	{
		while ( future_map_.size()>=unrun_ratio_*pool_size_+run_job_count_+pool_size_)
		{
			std::chrono::milliseconds dura( 200 );
			std::this_thread::sleep_for ( dura );
		}
		int post_index;
		std::shared_ptr < std::promise <size_t> > sptr ( new ( std::promise <size_t> ) );
		{
			std::lock_guard<std::mutex> mylock2 (my_mute2);
			future_map_.push_back ( std::move ( sptr->get_future() ) );
			post_index = future_map_.size()-1;
		}
		if ( CheckDependency (dependency_table) )
			PostImpl<JOB> ( std::forward<JOB&&>(job), sptr );
		else
			RePost (job, dependency_table, sptr );
		return post_index;
	}

	template <typename JOB>
	void RePost (JOB&& job, const std::vector<size_t>& dependency_table, std::shared_ptr < std::promise <size_t> >& sptr)//, int post_index=-1)
	{
		if ( CheckDependency (dependency_table) )
			PostImpl<JOB> (std::forward<JOB&&> (job), sptr );
		else
			io_service_.post ( std::bind (&ThreadPool//IOBound_update
					::RePost<JOB>, this, 
					std::forward<JOB&&>(job), std::forward< const std::vector<size_t> > (dependency_table), sptr) );//, post_index ) );
	}

	void FlushPool(void)
	{
		std::lock_guard<std::mutex> new_lock (my_mute2);

		for ( auto i = 0; i != future_map_.size(); ++i)
		{
			if ( future_map_[i].valid() )
				(future_map_[i]).wait();
			//for future ever calling its mem function wait_for (), it may leave its shared state, though it is taught that only get() and share () can 
			//have a future leaving its shared state. As such, we have to check whether this future is still valid () before wait (), otherwise undefine behavior
			//will occur
			else
				continue;
		}

		for ( auto& idv : brutal_future_map_ )
		{
			if ( idv.valid() )
				idv.wait();
			else
				continue;
		}
	}
	
	void FlushOne (size_t i)
	{
		std::lock_guard<std::mutex> new_lock (my_mute2);
//		if ( future_map_[i].valid() )
//			(future_map_[i]).wait();
		FlushOne (i, 5566);
	}

	void FlushOne (size_t i, int a)
	{
		if ( future_map_[i].valid() )
			(future_map_[i]).wait();
	}

	size_t GetFutureMapSize (void)
	{
		return future_map_.size();
	}

	std::vector < std::future<size_t> > brutal_future_map_;

	void BrutalFlushOne (size_t i)
	{
		std::lock_guard<std::mutex> new_lock (my_mute2);
		if ( brutal_future_map_[i].valid() )
			(brutal_future_map_[i]).wait();
	}

	template < typename JOB >
	int BrutalJobPost ( JOB&& job )
	{
		int post_index;
		std::shared_ptr < std::promise <size_t> > sptr ( new ( std::promise <size_t> ) );
		{
			std::lock_guard<std::mutex> mylock2 (my_mute2);
			brutal_future_map_.push_back ( std::move ( sptr->get_future() ) );
			post_index = brutal_future_map_.size()-1;
		}
		brutal_function_wrapper func_wrap;
		threads_.create_thread ( std::bind (func_wrap, std::forward<JOB&&>(job), sptr));
		return post_index;
	}
};

ThreadPool GlobalPool ( 10 );

#endif
