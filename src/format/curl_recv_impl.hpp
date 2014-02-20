/**
 *  @file curl_recv_impl.hpp
 *  @brief the provide basic implementation detail for data receiving through curl
 *  @author C-Salt Corp.
 */
#ifndef CURL_RECV_IMPL_
#define CURL_RECV_IMPL_
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/iostreams/concepts.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/pipeline.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "curl/curl.h" 
#include "../constant_def.hpp"

/**
 * @class curl_recv_exception
 * @brief define an exception class, public inherited from std::exception and thrown in CurlRecvImpl::recv_impl function
 */
class curl_recv_exception: public std::exception
{
public:
  virtual const char* what() const throw()
  {
      return "curl_recv exception happened";
  }
} curl_recv_exception_object;

std::mutex QQQ;
uint64_t curl_buf_size = 30*1024*1024;
uint64_t second_buf_size = 1*1024*1024;

/**
 * @brief define a global function served as the callback function needed for CurlRecvImpl::recv_impl function to facilitate the data receiving from curl to local buffer.  The parameter/return value interface of this function matches what is required by the callback function interface of the curl API in C.
 * @tparam MUT a pointer to std::mutex object, served as a non-type parameter in the template interface to pass a std::mutex object, which is competed by two threading functions, the current call_back_operate_mutex function for data receiving from curl to buffer, and the parsing function of the CurlWrapper class for data retrieving from buffer to downstream process.
 * @tparam FUNC a functor implementing the threading function of the call_back_operate_mutex function.
 */
template < std::mutex* MUT, typename FUNC >
size_t call_back_operate_mutex ( void* buf, size_t size, size_t nmemb, void* userp )
{
	FUNC opr (MUT);
	return opr (buf, size, nmemb, userp);
};

/**
 * @class curl_default_handle_mutex 
 * @brief provide the implementation for the callback function
 */
class curl_default_handle_mutex
{
	std::mutex* mut_;
	public:
/**
 * @brief constructor 
 */
	curl_default_handle_mutex (std::mutex* mutin)
		: mut_ (mutin)
	{}

/**
 * @brief functor implementation, share the interface of the callback function to handle the operation of data receiving from curl, i.e. the write back data structure userp, to local buffer, buf, wherein buf is managed by curl API and userp is actually passed in with a std::stringstream*. \n This operator() further manages the length of userp between a soft upperbound and a soft lowerbound, respectively assigned with global variables curl_buf_size, and second_buf_size, by means of manipulating the get and put stream pointers of userp. 
 */
// get_ptr -> ----------- 0 Mb                         -----------                  get_ptr -> ----------- 0 Mb 
//            |         |                              |         |                             |/////////|
//            |         |                              |         |                             ----------- second_buf_size 
//            |         |                =>            |         |               => put_ptr -> |         |
//            |         |                   get_ptr -> ----------- second_buf_size                 |         | 
//            |         |                              |/////////|                             |         |
// put_ptr -> ----------- curl_buf_size     put_ptr -> ----------- curl_buf_size               -----------
//
//  while ( put_ptr > curl_buf_size ), wait till the ceriterion met (put_ptr-get_ptr < second_buf_size), i.e. get_ptr catching up with put_ptr
//  after the above mentioned ceriterion met, clear the content of userp by 1) copying the remaining content of userp, 2) clear userp, 3) have the copied content put back to userp, and 4) reset get_ptr and put_ptr.
	size_t operator() (void* buf, size_t size, size_t nmemb, void* userp)
	{
		if (userp)
		{
			int userp_g, userp_p;
			std::stringstream& os = *static_cast<std::stringstream*>(userp);
			{
				std::lock_guard<std::mutex> l (*mut_);//(QQQ);
				userp_g = os.tellg (), userp_p = os.tellp ();
			}
			if ( userp_p > curl_buf_size )
			{	//put_ptr reaches the curl_buf_size, then waiting till 
				//get_ptr to catch up and criterion put_ptr-get_ptr < second_buf_size
				while (userp_p - userp_g >= second_buf_size )
				{
					std::chrono::milliseconds dura ( 100 );
					std::this_thread::sleep_for ( dura );
					userp_g = os.tellg ();
					userp_p = os.tellp ();
				}
				{	//when the criterion is met, having userp reset with the residue bewteen the put_ptr and get_ptr
					std::lock_guard<std::mutex> l(*mut_);
					userp_g = os.tellg();
					userp_p = os.tellp();
					int os_length = userp_p-userp_g;
					char* ka = new char [os_length];
					{
						os.read (ka, os_length);
						os.str("");
						os.clear();
						os.seekp(0);
						os.write (ka, os_length);
						delete [] ka;
					}
				}
			}
			std::streamsize len = size * nmemb;
			{	//conduct the operation of copying curl recv data into buf 
				std::lock_guard<std::mutex> l(*mut_);
				if ( os.write (static_cast<char*>(buf), len) )
					return len;
			}
		}
		return 0;
	}
};

/**
 * @class CurlRecvImpl 
 * @brief provide curl data receiving implementation. 
 * @tparam M a pointer to std::mutex object, served as a non-type parameter for passing in the std::mutex object competed by threading functions of call_back_operate_mutex functor and parsing function of CurlWrapper class.  
 * @tparam FUNC a type parameter indicating the type of the call_back functor called withing the global call_back_operate_mutex function, defaulted as the above mentioned curl_default_handle_mutex
 */
template < std::mutex* M, typename FUNC = curl_default_handle_mutex >
class CurlRecvImpl
{
private:
/**
 * @brief the data structure pointer passed in to carry the write data written by WRITEFUNCTION, i.e. the call_back function.  The data structure is in the type of std::stringstream, and is managed as shared_ptr object.
 */
	std::shared_ptr < std::stringstream > myos;

public:
/**
 * @brief constructor
 */
	CurlRecvImpl (void)
		: myos ( new std::stringstream )
	{
		std::string temp;
		temp.reserve (curl_buf_size);
		myos->str( temp );
	}

/**
 * @brief provide the main curl receiving function by calling APIs provided by curl library, wherein curl operation information, such as url address, write_callback function, write data structure, http header, and other related setting is determined.
 */
	uint64_t recv_impl (const std::string url, const std::string acquiring_range="", long timeout = 30)
	{
		CURLcode code (CURLE_FAILED_INIT);
		CURL* curl = curl_easy_init();

		curl_slist * pHeaders = NULL;
		const char* range = acquiring_range.c_str();
		pHeaders = curl_slist_append (pHeaders, range);

		try
		{
			if (curl)
			{
				if (	   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, url.c_str() ) ) 
						//indicating the url address for curl receiving operation
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, (call_back_operate_mutex <(std::mutex*)M, FUNC> ) ) )
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(*myos) ) ) 
						//write back function & data structure indicating how the operation of curl receiving is conducted
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOPROGRESS, 1L) ) 
						//shutting curl operation progress function off	
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_FOLLOWLOCATION, 1L) ) 
						//indicating curl to redirect and follow any location
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) )  
						//mannually set the httpheader to facilitate the transmission in part function, e.g. adding http header like Range: bytes=0-1024
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOSIGNAL, 1) ) 
						//for threading purpose 
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
						//turn SSL validation check off
						&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1) )
						//&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_TIMEOUT, timeout ) )
					//	&& CURLE_OK == ( code =  curl_easy_setopt ( curl, CURLOPT_WRITEHEADER, headerfile) )
				   )
				{
					code = curl_easy_perform (curl);
					std::cerr<<"curl recv easy perform "<<curl_easy_strerror(code)<<std::endl;
					if (code != CURLE_OK)
						throw curl_recv_exception_object;
				}
				else
				{
					std::cerr<<"curl recv exception not CURLE_OK "<<curl_easy_strerror(code)<<std::endl;
					throw curl_recv_exception_object;
				}
				curl_easy_cleanup (curl);
			}
			else
			{
				std::cerr<<"curl init exception CURLcode "<<curl_easy_strerror(code)<<std::endl;
				throw curl_recv_exception_object;
			}
		}
        catch (std::exception& excp)
        {
            std::cerr<<"curl_recv exception catched "<<excp.what()<<std::endl;
            double str;
            code = curl_easy_getinfo ( curl, CURLINFO_SIZE_DOWNLOAD, &str );
            uint64_t length_r = (uint64_t) str;
			curl_easy_cleanup (curl);
            return length_r;
        }
        double str;
		code = curl_easy_getinfo ( curl, CURLINFO_SIZE_DOWNLOAD, &str );
		uint64_t length_r = (uint64_t) str;
		std::cerr<<"curl recv before return "<<curl_easy_strerror(code)<<std::endl;
		return length_r;
	}

/**
 * @brief provide a public access function to return a shared_ptr object pointing to the write back data structure std::stringstream 
 */
	std::shared_ptr <std::stringstream> get_result (void)
	{
		return myos;
	}
};

void INITIALIZE_CURL (void)
{
	curl_global_init (CURL_GLOBAL_ALL);	
};
#endif
