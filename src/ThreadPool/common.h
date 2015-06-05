#ifndef THREAD_POOL_COMMON_H
#define THREAD_POOL_COMMON_H 
#include <mutex>
//#include "../debugger/logger.hpp"
template<class T, class F, class... Arg>
void obj_lock_access(T&& obj, F&& op, Arg&&... args)
{
	static std::mutex m;
	std::lock_guard<std::mutex> lock(m);
	op(obj, args...);
}
class ThreadAdapter;
class ThreadPool;
#include "thread_job.hpp"
#include "thread_adapter.h"
#include "thread_pool.h"
#endif
