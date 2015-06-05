#ifndef THREAD_JOB_HPP
#define THREAD_JOB_HPP 
#include <vector>
#include <functional>

struct ThreadJob
{
	size_t job_id;
 	std::vector<size_t> wait_job_ids;

 	template < typename JOB_func >
 	ThreadJob
 	(
 		JOB_func&& job_content_proto, 
 		const std::vector<size_t>& wait_job_ids
 	)
 	: wait_job_ids		(wait_job_ids)
 	, job_content_proto (job_content_proto)
 	{}

 	inline std::function <void()> job_dispatch()
 	{
 		return job_content_proto;
 	}
 	
private:
 	std::function<void()> job_content_proto;

};

#endif
