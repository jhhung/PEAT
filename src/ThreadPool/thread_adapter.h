#ifndef THREAD_ADAPTER_H
#define THREAD_ADAPTER_H

#include "common.h"
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadAdapter
{	
friend ThreadPool;
private: // private operation should not lock the state
	enum States { 
        ONLINE, //passtive
        OFFLINE, //passive
        FLUSHING, //active
        IDLE //active
    };


	std::mutex 				thread_state_mutex;//write in thread read in controller
	volatile States 		thread_state;
	std::queue<ThreadJob> 	job_queue;
    std::mutex				job_queue_mutex;
	std::thread* 			worker;
	ThreadPool* 			attribution_pool;
	std::condition_variable queue_full_cv;
	std::mutex 				queue_full_cv_mutex;
	int      				queue_max_buffer;
	int 					tid;
	void load_worker();
    void job_queue_safe_push(ThreadJob& job);
    void send_finish_request();
    void release_worker();
public:
	enum AssignFail { BUFFER_FULL, THREAD_FLUSH};
	ThreadAdapter(ThreadPool* pool, int pool_load_tid, int queue_max_buffer = 100);
	ThreadAdapter(const ThreadAdapter& thread_adapter) = delete;
	ThreadAdapter();
    //ThreadAdapter(ThreadAdapter&& thread_adapter);
	~ThreadAdapter();
    void finish_queue();
	void assign_job(ThreadJob& job);
	void join();
};
#endif
