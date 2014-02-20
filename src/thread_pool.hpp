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
	void operator() ( FUNC&& func, std::shared_ptr < std::promise <size_t> > sptr )
	{
		func ();
		sptr -> set_value (0);
	}

	template < typename FUNC >
	void operator() ( FUNC&& func, std::shared_ptr < std::promise <size_t> > sptr,
						std::shared_ptr < std::promise <size_t> > sptr2 )
	{
		func ();
		sptr -> set_value (0);
		sptr2 -> set_value (0);
	}
};

/// @class ThreadPool
/// @brief Include boost:::asio::io_service, thread_group, and asio::io_service_work member elements for taking care of thread pool management \n NOTE: PROPERLY EMPLOYTHE FLUSH OPERATION, i.e. the operation of gathering parallel processing results, is crucial in the world of parallel processing, especially in the context that sequential intercorrelation is presented among input/output interfaces of each parallel processings
/// 
class ThreadPool
{
//private:
public:
/// @brief ThreadPool element, in the type of boost::asio::io_service, designed to provide a stronger guarantee that it is safe to use a single object concurrently.  The io_srvice_ object is used to facilitate the concept of ThreadPool and the management invoked thereof
	boost::asio::io_service io_service_;
/// @brief ThreadPool element, providing a collection of threads related in some fashion.  New threads can be added to the thread_group object with either add_thread or create_thread member functions
	boost::thread_group threads_;
/// @brief ThreadPool element, in the type of boost::asio::io_service::work, which is constructed with an asio::io_service object, and is used to inform io_service that work starts and finishes, to ensure io_service's run() function will not exit while work is underway, and does exit when no unfinished work remaining \n A shared_ptr scheme is applied to manage the life time of the asio::io_service::work object.  
	boost::shared_ptr < boost::asio::io_service::work > work_; 
/// @brief ThreadPool elements, holding future object return from a promise object, that is used to indicate whether a posted job has returned or not.  future_vec is also served as a job-returned-indicator in the FlushPool function.

	std::vector< std::future <size_t> > future_vec_;
	std::vector < std::pair < size_t, std::future <size_t> > > future_map_;

	size_t current_pool_size_;
/// @brief ThreadPool elements, representing threadpool size
	size_t pool_size_;
/// @brief ThreadPool elements, holding future object return from a promise object, that is used to indicate whether a posted job has returned or not.  future_vec is also served as a job-returned-indicator in the FlushPool function.
public:
/// @brief Constructor, taking a pool_size of size_t format, and accordingly creating that much of threads \n
/// elements work_, a pointer pointing to asio::io_service_work element, is initialized as a shared_ptr
	ThreadPool ( std::size_t pool_size )
		: work_ ( new boost::asio::io_service::work ( io_service_ ) )
		, pool_size_ ( pool_size )
		, current_pool_size_ (0)
	{
		for ( std::size_t i = 0; i < pool_size; ++i )
			threads_.create_thread ( boost::bind ( &boost::asio::io_service::run, &io_service_ ) );
	}
/// @brief Member function for altering pool_size_ 
	void ChangePoolSize ( std::size_t new_pool_size)
	{
		pool_size_ = new_pool_size;
		FlushPool();
	}
/// @brief Member function, old and slower version of flush, with thread destroy and reborn. 
	void FlushPool (size_t i)
	{
		work_.reset();			
		threads_.join_all();	// have all threads terminated to gather the corresponding results
		io_service_.reset();	// reset io_service_ object for future invocation of boost::asio::io_service::run(), run_one(), poll(), or poll_one()
		work_.reset( new boost::asio::io_service::work( io_service_ ) );
		for ( std::size_t i = 0; i < pool_size_; ++i )
			threads_.create_thread ( boost::bind ( &boost::asio::io_service::run, &io_service_ ) );
	}
/// @brief Destructor, with operation killing asio::io_service::work object and threads 
	~ThreadPool ()
	{
		work_.reset();
		threads_.join_all();
	}
/// @brief Main operation function, realized with boost::asio::io_service object's member function post, to register the to-be-handled job on the io_service object's working queue.
	template < typename JOB >
	void JobPost ( JOB&& job ) //&& here is actually an "universal reference" acording to Scott Meyers, and it can be deducted as both r and l refrence formats.
	{
		set_pool_index ();
		std::shared_ptr < std::promise <size_t> > sptr ( new ( std::promise <size_t> ) );
		future_map_[current_pool_size_] = std::move ({ current_pool_size_, sptr->get_future() });
//		future_map_.push_back ({current_pool_size_, sptr->get_future()});

		function_wrapper func_wrap;
		io_service_.post ( std::bind (func_wrap, std::forward<JOB&&>(job), sptr ) );//, current_pool_size_) ) ;
//		std::shared_ptr < std::promise <size_t> > sptr ( new ( std::promise <size_t> ) );
//		future_vec_.push_back (sptr->get_future());
//		function_wrapper func_wrap;
//		io_service_.post ( std::bind (func_wrap, std::forward<JOB&&>(job), sptr) ) ;
	}
/// @brief Member function, with operation having current asio::io_service::work object and threads reset
	void FlushPool()
	{
//		for ( auto i=0; i!=future_vec_.size(); ++i)
//			future_vec_[i].wait();
//		future_vec_.clear();
		for ( auto i = 0; i != future_map_.size(); ++i)
		{
			if ( future_map_[i].second.valid() )
				(future_map_[i]).second.wait();
			else
				continue;
		}
		future_map_.clear();
	}

	size_t set_pool_index (void)
	{
		if ( future_map_.size() < pool_size_ )
		{
			current_pool_size_ = future_map_.size();
			return current_pool_size_;
		}
		else
		{
			while (true)
			{
				for ( auto i = future_map_.rbegin(); i != future_map_.rend(); ++i ) 
				{
					if ( i -> second.wait_for ( std::chrono::microseconds (1) ) == std::future_status::ready )
					{
					  	current_pool_size_ = i -> first;
						return current_pool_size_;
					}
				}
			}
		}
	}

	size_t get_queue_size ()
	{
		return future_map_.size();//future_vec_.size();
	}

	void single_flushPool (size_t i)
	{
		(future_map_[i]).second.wait();
	}

};

/// @brief A global object, as the default thread pool object
ThreadPool GlobalPool ( 10 );

#endif
