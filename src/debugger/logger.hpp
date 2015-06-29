#ifndef LOGGER
#define LOGGER 

#include <string>
#include <iostream>
#include <thread>
#include <map>
#include <chrono>
#include <mutex>
#include <tuple>
#include <vector>

using namespace std::chrono;
//#define DEBUG_timer 1


template<typename... T>
inline void err_printf(const char* format, T... var)
{
	#if DEBUG_log == 1
	fprintf(stderr, format, var...);
	#endif 
}


template<typename T>
inline void lock_cerr(T&& obj);



struct Log_err_L3
{	
	template <typename T>
	inline friend Log_err_L3& operator << (Log_err_L3& tracer, T&& obj)
	{
		#if DEBUG_log == 3
		lock_cerr(obj);
		#endif
		return tracer;
	}
};
//Log_err_L3 logerr3;
//Log_err_L3 logerr;
struct Log_err_L1
{	
	template <typename T>
	inline friend Log_err_L1& operator << (Log_err_L1& tracer, T&& obj)
	{
		#if DEBUG_log == 1
		lock_cerr(obj);
		#endif
		return tracer;
	}
};
//Log_err_L1 logerr1;
/*
struct TimeAdapter
{
	enum CLOCK_FLAG
	{
		start, pause, resume, stop
	};
	typedef std::tuple<std::string, steady_clock::time_point, CLOCK_FLAG> 	TIME_POINT;
	typedef std::vector<TIME_POINT> 										TIME_POINT_STACK;
	typedef std::map<std::thread::id, TIME_POINT_STACK>						CLOCKS;
	static 	CLOCKS 															clocks;

	inline static void clock_start(std::string clock_topic)
	{
	#if DEBUG_timer == 1
		TimeAdapter::clock_pause();
		std::thread::id tid = std::this_thread::get_id();
		if (clocks.find(tid) != clocks.end())
		{
			clocks[tid].push_back(
				make_tuple(
					clock_topic,
					steady_clock::now(),
					CLOCK_FLAG::start
				)
			);
		}
		else
		{
			clocks.emplace(
				tid, 
				std::vector<TIME_POINT> ({ 
					make_tuple(
						clock_topic,
						steady_clock::now(),
						CLOCK_FLAG::start
					)
				}) 
			);
		}
		TimeAdapter::clock_resume();
	#endif
	}
	inline static void clock_pause()
	{
	#if DEBUG_timer == 1
		//TimeAdapter::clock_pause();
		std::thread::id tid = std::this_thread::get_id();
		if (clocks.find(tid) != clocks.end())
		{
			clocks[tid].push_back(
				make_tuple(
					"",
					steady_clock::now(),
					CLOCK_FLAG::pause
				)
			);
		}
		else
		{
			clocks.emplace(
				tid, 
				std::vector<TIME_POINT> ({ 
					make_tuple(
						"",
						steady_clock::now(),
						CLOCK_FLAG::pause
					)
				}) 
			);
		}
		//TimeAdapter::clock_resume();
	#endif
	}

	inline static void clock_resume()
	{
	#if DEBUG_timer == 1
		//TimeAdapter::clock_pause();
		std::thread::id tid = std::this_thread::get_id();
		if (clocks.find(tid) != clocks.end())
		{
			clocks[tid].push_back(
				make_tuple(
					"",
					steady_clock::now(),
					CLOCK_FLAG::resume
				)
			);
		}
		else
		{
			clocks.emplace(
				tid, 
				std::vector<TIME_POINT> ({ 
					make_tuple(
						"",
						steady_clock::now(),
						CLOCK_FLAG::resume
					)
				}) 
			);
		}
		//TimeAdapter::clock_resume();
	#endif
	}

	inline static void clocks_end(bool function_tree_trace = true)
	{
	#if DEBUG_timer == 1
		TimeAdapter::clock_pause();
		auto tmp = steady_clock::now();
		std::thread::id tid = std::this_thread::get_id();
		if (clocks.find(tid) != clocks.end())
		{
			std::vector<duration<double>> time_spans;
			while( std::get<2>(clocks[tid].back()) !=  CLOCK_FLAG::start)
			{
				switch (std::get<2>(clocks[tid].back()))
				{
					case CLOCK_FLAG::resume : time_spans.push_back( tmp - std::get<1>(clocks[tid].back()) ); 
						break;
					case CLOCK_FLAG::pause : tmp = std::get<1>(clocks[tid].back()); 
						break;
					default : break; // FIX : add error message for other CLOCK_FLAG
				}
				clocks[tid].pop_back();
			}
			
			duration<double> time_span(0.0);
			for ( duration<double> span : time_spans)
				time_span += span ;
			time_span += (tmp - std::get<1>(clocks[tid].back()));

			std::string clock_topic = "";
			if(function_tree_trace)
			{
				for (TIME_POINT_STACK::iterator i = clocks[tid].end() - 1; i != clocks[tid].begin() - 1; --i)
				{
					if (std::get<2>(*i) == CLOCK_FLAG::start)
					{
						clock_topic = std::get<0>(*i) + " :: " + clock_topic;
					}
					
				}
			}
			else
			{
				clock_topic = std::get<0>(clocks[tid].back());
			}
			clocks[tid].pop_back();
			lock_cerr ( "[" + clock_topic + "] : " + std::to_string(time_span.count()) + " seconds.\n" );
			 
		}
		else
		{
			lock_cerr ("warnning : timmer error, tid has no match stack.\n");
		}
		TimeAdapter::clock_resume();
	#endif
	}
};
TimeAdapter::CLOCKS TimeAdapter::clocks;
*/
template<typename T>
inline void lock_cerr(T&& obj)
{
	//TimeAdapter::clock_pause();
	static std::mutex out_mutex;
	std::lock_guard<std::mutex> lock(out_mutex);
	std::cerr << obj;
	//TimeAdapter::clock_resume();
}
#endif
