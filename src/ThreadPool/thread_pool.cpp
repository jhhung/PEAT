#include <thread>
#include <vector>
#include <queue>
#include <utility>
#include <functional>
#include <mutex>
#include "ThreadPool/common.h"
#include "debugger/logger.hpp"
#include <iostream>
#include <chrono>
#include <memory>
int ThreadPool::get_next_jid() // override this if you want to change the schedule algorithm
{
	std::lock_guard<std::mutex> id_lock(job_id_counter_mutex);
	job_id_counter++;
	return job_id_counter;
}

void ThreadPool::job_post_to(ThreadJob job, ThreadAdapter& thread_adapter)
{
	//err_printf("postting jid : %d to thread : %d\n", job.job_id, thread_adapter.tid);
TRY_POST:
	try
	{
		thread_adapter.assign_job(job);
	}
	catch(ThreadAdapter::AssignFail a_f)
	{
        goto TRY_POST;
		//err_printf("post jid : %d to thread : %d...fail, code : %d.\n", job.job_id, thread_adapter.tid, a_f);
	    //abort();
	}
	
	
}

ThreadAdapter* ThreadPool::get_thread(int jid) // override this if you want to change the schedule algorithm
{
	{
		std::lock_guard<std::mutex> lock(idle_list_mutex);
		if (!idle_list.empty()) //never use...
		{
			//err_printf("took thread from idlelist!!!\n");
			ThreadAdapter* idle_thread = idle_list.front();
			idle_list.pop();
			return idle_thread;
		}
	}
	return &thread_set[ jid % max_thread_number];
}
ThreadAdapter* ThreadPool::get_thread(int jid, int skip_tid) // override this if you want to change the schedule algorithm
{
	{
		std::lock_guard<std::mutex> lock(idle_list_mutex);
		if (!idle_list.empty()) //never use...
		{
			ThreadAdapter* idle_thread = idle_list.front();
			idle_list.pop();
			return idle_thread;
		}
	}
	int tid = jid % max_thread_number;
	return &thread_set[ (tid + (skip_tid == tid)) % max_thread_number];
}
void ThreadPool::initialize_thread_set()
{
    int q_buffer(max_job_count / max_thread_number);
	for(int i = 0; i < max_thread_number; i++)
    {
		thread_set[i].tid = i;
        thread_set[i].queue_max_buffer = q_buffer; 
        thread_set[i].attribution_pool = this;
    }

}
void ThreadPool::reset_dependency_list()
{
    for(int i = 0; i < max_job_count; i ++) 
    {
        std::lock_guard<std::recursive_mutex> lock(*std::get<2>(wait_dependency_list[i]));
        auto&& single_list(wait_dependency_list[i]);
        std::get<0>(single_list) = false;
        std::get<1>(single_list).clear();
    }
}
ThreadPool::ThreadPool(int max_thread_num, int max_job_count )
	: max_thread_number(max_thread_num)
	, thread_set( max_thread_num )
	, job_id_counter(-1)
	, max_job_count(max_job_count)
	, wait_dependency_list(
		max_job_count,//max dependency wait (parameter)
		std::tuple<bool, std::list<ThreadJob>, std::recursive_mutex*>(
		    false, {}, new std::recursive_mutex
		)
	  )
	, flush_flag(false)
    , job_id_round(-1)

{
    initialize_thread_set();
}
ThreadPool::~ThreadPool()
{
    thread_set.clear();
   //err_printf("thread_set cleared.\n");
    for(auto&& e : wait_dependency_list)
    {
        if(std::get<2>(e)->try_lock())
        {
            std::get<2>(e)->unlock();
            //delete std::get<2>(e);
            //err_printf("deleted thread mutex.\n");
        }
    }
    wait_dependency_list.clear();
}
void ThreadPool::job_free_dependency(int jid, int caller_tid)//FIXME:remove caller_tid
{
	//err_printf("job_free_dependency : %d\n", jid);
	std::lock_guard<std::recursive_mutex> lock(*std::get<2>(wait_dependency_list[jid % max_job_count]));
	std::get<0>(wait_dependency_list[jid % max_job_count]) = true;
	//err_printf("job_free_dependency : %d...check\n", jid);
	for(auto& job : std::get<1>(wait_dependency_list[jid % max_job_count]))
	{
		if(check_dependency(job.wait_job_ids))
		{
		   //err_printf("re post job, jid : %d\n", job.job_id);
			int offset_num = 0;
			job_post_to(job, *get_thread(job.job_id + offset_num));
		}
        else
        {
            //FIXME : haven't test this scope
            for(size_t wait_job_id : job.wait_job_ids)
            {
                std::get<1>( wait_dependency_list[wait_job_id % max_job_count] ).push_back(job);
                //err_printf("jid : %d, push to wait queue\n", job.job_id);
            }	

        }
	}
	//err_printf("job_free_dependency : %d...end\n", jid);
}

int ThreadPool::job_post(ThreadJob& job)
{
    std::lock_guard<std::mutex> lock(flush_mutex);//flush 
	int jid = get_next_jid();
    if(jid % max_job_count == 0 )//first job in round
    {
	   //err_printf("new round start\njid : %d post to new round\n", jid);
        if(job_id_round > -1)// not the first round
            wait_job_list_finish();
        reset_dependency_list();
        {
            std::lock_guard<std::mutex> lock(job_id_counter_mutex);
            job_id_round ++;
        }
    }
	job.job_id = jid;
	if(check_dependency(job.wait_job_ids))//true if no dependency
	{
        int offset_num = 0;
        job_post_to(job, *get_thread(jid+ offset_num));
	}
	else
	{
		//std::lock_guard<std::mutex> lock(wait_dependency_list_mutex);
		for(size_t wait_job_id : job.wait_job_ids)
		{
            std::lock_guard<std::recursive_mutex> lock(*std::get<2>( wait_dependency_list[wait_job_id % max_job_count] ));
			std::get<1>( wait_dependency_list[wait_job_id % max_job_count] ).push_back(job);
		//err_printf("jid : %d, push to wait list\n", job.job_id);
		}	
		
	}
	return jid;
}

bool ThreadPool::check_dependency(std::vector<size_t>& wait_job_ids) //return true if nodependecy else return false
{
    //err_printf("wait_job_ids.size() : %d\n", wait_job_ids.size());
    //std::lock_guard<std::mutex> lock(wait_dependency_list_mutex);
	if(wait_job_ids.size() != 0)
	{
		for(size_t wait_job_id : wait_job_ids)
        {
            if(wait_job_id >= (job_id_round * max_job_count))
            {
                std::lock_guard<std::recursive_mutex> lock(*std::get<2>(wait_dependency_list[wait_job_id % max_job_count]));
			    if(!std::get<0>(wait_dependency_list[wait_job_id % max_job_count])) 
                    return false;
            }
        }
	}
	return true;
}

void ThreadPool::wait_job_list_finish()
{
   //err_printf("wait_job_list_finish\n");
    //std::cerr << "wait_job_list_finish\n";
	for (int i = 0; i < max_job_count; i++)
	{
       //err_printf("dep %d\n", i );
		while(!std::get<0>(wait_dependency_list[i]))
		{
            //std::this_thread::sleep_for(std::chrono::milliseconds(300));
			std::this_thread::yield();
		}
			
	}

}

void ThreadPool::wait_job_list_finish_last()
{
   //err_printf("wait_job_list_finish\n");
    //std::cerr << "wait_job_list_finish\n";
	for (int i = 0; i <= job_id_counter % max_job_count; i++)
	{
       //err_printf("dep %d\n", i );
		while(!std::get<0>(wait_dependency_list[i]))
		{
            //std::this_thread::sleep_for(std::chrono::milliseconds(300));
			std::this_thread::yield();
		}
			
	}

}

void ThreadPool::flush_pool()
{
	//err_printf("start flush_pool\n");
	//flush_flag = true;//TODO :this variable should have a lock
    std::lock_guard<std::mutex> lock(flush_mutex);
    wait_job_list_finish_last();
//err_printf("all job distributed.\n");
	for(ThreadAdapter& ta : thread_set)
	{
		//err_printf("thread : %d, start join\n", ta.tid);
		ta.join();
		//err_printf("thread : %d, end join\n", ta.tid);
	}
    reset_dependency_list();
    //err_printf("end flush_pool\n");
}

void ThreadPool::FlushPool(void)
{
	flush_pool();
}

void ThreadPool::FlushOne (size_t i)
{
//	thread_set[i].finish_queue();
//	thread_set[i].join();
//    std::cerr << job_id_round << std::endl;

    if(i >= (job_id_round * max_job_count))
    {
        while(!std::get<0>(wait_dependency_list[i % max_job_count])) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            //std::this_thread::yield();
        }
    }

}
void ThreadPool::change_pool_size(size_t new_size)
{
    max_thread_number = new_size;
    reset_pool();
}
void ThreadPool::reset_pool(void)
{
//    typedef std::vector<ThreadAdapter> ThreadContainer;
    //FIXME : havent test.
    flush_pool();
    if(max_thread_number != thread_set.size() )
    {
        ThreadContainer tmp(max_thread_number);
        thread_set.swap(tmp);
        initialize_thread_set();
    }
    //TODO : idle list check (idle_list working draft)
}
void ThreadPool::ChangePoolSize(size_t new_size)
{
    change_pool_size(new_size);
}
void ThreadPool::ResetPool(void)
{
    reset_pool();
}
ThreadPool GlobalPool ( 10 );
