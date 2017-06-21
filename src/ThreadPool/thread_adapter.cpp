#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <iostream>
#include "ThreadPool/common.h"
#include "debugger/logger.hpp"
#include <stdexcept>
void ThreadAdapter::load_worker()
{
    //err_printf("tid : %d loading thread\n", tid);
    //std::lock_guard<std::mutex> lock(thread_state_mutex); 
    if( thread_state != OFFLINE || worker != NULL )
    {
        std::cerr << "worker load check error." << std::endl;
        abort();
        //throw std::logic_error("worker load check error.");
    }
    else
    {
	    worker = new std::thread([this]()
	    {
	    	while((thread_state != FLUSHING) || !job_queue.empty())
	    	{
	    		if(!job_queue.empty())
	    		{
	    			ThreadJob* job (&job_queue.front());
	    			job->job_dispatch()();
                    int free_jid = job->job_id;
                    {
                        std::lock_guard<std::mutex> lock(job_queue_mutex);
                        job_queue.pop();
                        queue_full_cv.notify_one();
                    }
                    if(free_jid != -1)
                    {
                        attribution_pool->job_free_dependency(free_jid, this->tid);
                    }
	    			//////err_printf("jid : %d finish\n", job.job_id);
	    		}
	    		else
	    		{
	    			std::this_thread::yield();
	    		}
	    	}
	    });
        thread_state = ONLINE;
        //err_printf("tid : %d ONLINE\n", tid);
    }
	
}


ThreadAdapter::ThreadAdapter(ThreadPool* pool, int pool_load_tid, int queue_max_buffer)
: thread_state		(OFFLINE)
, attribution_pool	(pool)
, worker			(NULL)
, tid				(pool_load_tid)
, queue_max_buffer 	(queue_max_buffer)
{}
ThreadAdapter::ThreadAdapter()
: thread_state		(OFFLINE)
, attribution_pool	(NULL)
, worker			(NULL)
, tid				(-1)
, queue_max_buffer 	(0)
{}
//
//ThreadAdapter::ThreadAdapter(const ThreadAdapter& thread_adapter) 
//: thread_state 			( thread_adapter.thread_state )	
//, job_queue 			( thread_adapter.job_queue )
//, worker 				( thread_adapter.worker )
//, attribution_pool		( thread_adapter.attribution_pool )
//, tid					( thread_adapter.tid )
//, queue_max_buffer		( thread_adapter.queue_max_buffer )
//{}
//ThreadAdapter::ThreadAdapter(ThreadAdapter&& thread_adapter) 
//: thread_state 			( thread_adapter.thread_state )	
//, job_queue 			( std::move(thread_adapter.job_queue) )
//, worker 				( thread_adapter.worker )
//, attribution_pool		( thread_adapter.attribution_pool )
//, tid					( std::move(thread_adapter.tid) )
//, queue_max_buffer		( std::move(thread_adapter.queue_max_buffer) )
//, thread_state_mutex	( std::move(thread_adapter.thread_state_mutex) )
//, job_queue_mutex	    ( std::move(thread_adapter.job_queue_mutex) )
//, queue_full_cv	        ( std::move(thread_adapter.queue_full_cv) )
//, queue_full_cv_mutex	( std::move(thread_adapter.queue_full_cv_mutex) )
//{}

ThreadAdapter::~ThreadAdapter()	//exculsive with assign_job
{
    //err_printf("tid : %d destroy\n", tid);
    thread_state_mutex.lock();
    switch(thread_state)
    {
        case ONLINE:
            send_finish_request();
            thread_state_mutex.unlock();
            if(worker->joinable())
            {
                worker->join();
            }
            release_worker();
            break;

        case OFFLINE:
            thread_state_mutex.unlock();
            break;

        case FLUSHING:
            thread_state_mutex.unlock();
            if(worker->joinable())
            {
                worker->join();
            }
            release_worker();
            break;

        case IDLE:
            send_finish_request();
            thread_state_mutex.unlock();
            if(worker->joinable())
            {
                worker->join();
            }
            release_worker();
            break;
    }
}

void ThreadAdapter::finish_queue()
{
	//std::lock_guard<std::mutex> lock(thread_state_mutex);
	//////err_printf("thread : %d, start join_finish_queue\n", tid);
	//if( thread_state == OFFLINE ) return;
	//else
	//{
	//	thread_state = FLUSHING;
	//}
}

void ThreadAdapter::send_finish_request()
{
    ThreadJob finish_job([this](){
        std::lock_guard<std::mutex> lock(thread_state_mutex);
       //err_printf("tid : %d get FLUSHING pack\n", tid);
        thread_state = FLUSHING;
    }, std::vector<size_t>(0));
    finish_job.job_id = -1;
    job_queue_safe_push(finish_job);
}

void ThreadAdapter::join()
{
    thread_state_mutex.lock();
    //err_printf("tid : %d state critical join\n", tid);
    if((worker == NULL && thread_state != OFFLINE) || (worker != NULL && thread_state == OFFLINE))
    {
        std::cerr << "thread join check error.\n";
        abort();
    }
    if(thread_state != OFFLINE)
    {
        if(worker->joinable())
        {
            send_finish_request();
            thread_state_mutex.unlock();
            worker->join();
            thread_state_mutex.lock();
        }
        release_worker();
    }
    thread_state_mutex.unlock();
}
void ThreadAdapter::release_worker()
{
    //err_printf("tid : %d release worker\n", tid);
    delete worker;
    worker = NULL;
    thread_state = OFFLINE;
}
void ThreadAdapter::job_queue_safe_push(ThreadJob& job)
{
    std::unique_lock<std::mutex> lock(queue_full_cv_mutex);//free
    queue_full_cv.wait(
        lock, 
        [this](){return job_queue.size() < queue_max_buffer;});
    {
    std::lock_guard<std::mutex> q_lock(job_queue_mutex);//free
    job_queue.push(std::move(job));
    }
}
void ThreadAdapter::assign_job(ThreadJob& job)
{
    {
	std::lock_guard<std::mutex> lock(this->thread_state_mutex);//free
    //err_printf("tid : %d state critical assign\n", tid);
    switch(thread_state)
    {
        case ONLINE:
            job_queue_safe_push(job);
            break;
        case OFFLINE://thread check state in OFFLINE-turn lock block 
            //when thread first birth and after join
            job_queue_safe_push(job);
			load_worker();//make worker online and start work
            break;
        case FLUSHING: // forbidden input any job, only after join and destroy
			throw THREAD_FLUSH;
            break;
        case IDLE: 
        //unfinished state, should be a state when worker have a long time waiting
            job_queue_safe_push(job);
            break;
    }
    }
}










