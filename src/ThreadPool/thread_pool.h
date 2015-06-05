#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "common.h"
#include <thread>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <forward_list>
#include <tuple>

class ThreadPool
{	
	friend ThreadAdapter;
	enum PoolException { POST_DURING_FLUSH };
	int 								max_thread_number;
	int 								job_id_counter;
    int                                 job_id_round;
	std::mutex							job_id_counter_mutex;
	std::vector<ThreadAdapter> 			thread_set;
	std::queue<ThreadAdapter*> 			idle_list;
	std::mutex							idle_list_mutex;

	std::vector<std::tuple<
		bool,
		std::forward_list<ThreadJob>
	>>									wait_dependency_list;
	std::mutex							wait_dependency_list_mutex;
	const int							max_job_count;
	bool 								flush_flag;
	std::condition_variable 			flush_cv;
	std::mutex 							flush_cv_mutex;
	
	
	int		get_next_jid();
	void	job_post_to(ThreadJob job, ThreadAdapter& thread_adapter);
protected:
	ThreadAdapter*
			get_thread(int jid);
	ThreadAdapter*
			get_thread(int jid, int skip_tid);
	bool	check_dependency(std::vector<size_t>& wait_job_ids);
    void    wait_job_list_finish();
    void    reset_dependency_list();
public:
	ThreadPool(int max_thread_num, int max_job_count = 10000);
	void	job_free_dependency(int jid, int caller_tid);
	int		job_post(ThreadJob& job);//only invoke by custom object
	void	flush_pool();
    void    change_pool_size(size_t new_size);
    void    reset_pool(void);
//template
	template < typename JOB_func> 
	int job_post(JOB_func&& func)
	{
		ThreadJob job(func, std::vector<size_t>(0));
		return job_post(job);
	}
	
	template < typename JOB_func > 
	int job_post
	(
		JOB_func&& func, 
		const std::vector<size_t>& dependent_jid
	)
	{
		ThreadJob job(func, dependent_jid);
		return job_post(job);
	}
//for pokemon compactable interface
	void 	FlushPool(void);
	void 	FlushOne (size_t i);//only invoke by custom object
    void    ChangePoolSize(size_t new_size);
    void    ResetPool(void);

    #ifndef JobPost
	    #define JobPost job_post
    #endif
};
extern ThreadPool GlobalPool;
#endif
