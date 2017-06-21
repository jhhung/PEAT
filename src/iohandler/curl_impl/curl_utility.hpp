#ifndef CURL_UTILITY_HPP_
#define CURL_UTILITY_HPP_

#include <mutex>
#include <thread>
#include <sstream>

class curl_send_exception: public std::exception
{                                                                                                                                                                                       
public:
    virtual const char* what() const throw()
    {  
        return "curl_send exception happened";
    }
}
curl_send_exception_object;

class curl_recv_exception: public std::exception
{
public:
  virtual const char* what() const throw()
  {
      return "curl_recv exception happened";
  }
} curl_recv_exception_object;
    
template < typename FUNC >
size_t call_back_operate_mutexless ( void* buf, size_t size, size_t nmemb, void* userp )
{ 
    FUNC opr;
    return opr (buf, size, nmemb, userp);
};  

class curl_default_handle//_json
{                                                                                                                                                                                       
public:
    size_t operator() (void* buf, size_t size, size_t nmemb, void* userp)
    {
        if (userp)
        {
            std::stringstream& os = *static_cast<std::stringstream*>(userp);
            std::streamsize len = size * nmemb;
            if ( os.write (static_cast<char*>(buf), len) )
                return len;
        }
        return 0;
    }
};  


std::mutex QQQ;
uint64_t curl_buf_size = 30*1024*1024;
uint64_t second_buf_size = 1*1024*1024;

template < std::mutex* MUT, typename FUNC >
size_t call_back_operate_mutex ( void* buf, size_t size, size_t nmemb, void* userp )
{                                                                                                                                                                                       
    FUNC opr (MUT);
    return opr (buf, size, nmemb, userp);
};

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
			{   //put_ptr reaches the curl_buf_size, then waiting till 
				//get_ptr to catch up and criterion put_ptr-get_ptr < second_buf_size
				while (userp_p - userp_g >= second_buf_size )
				{
					std::chrono::milliseconds dura ( 100 );
					std::this_thread::sleep_for ( dura );
					{
						std::lock_guard<std::mutex> l(*mut_);
						userp_g = os.tellg ();
						userp_p = os.tellp ();
					}
				}
				{   //when the criterion is met, having userp reset with the residue bewteen the put_ptr and get_ptr
					std::lock_guard<std::mutex> l(*mut_);
					userp_g = os.tellg();
					userp_p = os.tellp();
					uint64_t os_length = userp_p-userp_g;
					char* ka = new char [os_length+1];
					os.read (ka, os_length);
					os.str("");
					os.clear();
					os.seekp(0);
					os.seekg(0);
					os.write (ka, os_length);
					delete [] ka;
				}
			}
			std::streamsize len = size * nmemb;
			{   //conduct the operation of copying curl recv data into buf 
				std::lock_guard<std::mutex> l(*mut_);
				if ( os.write (static_cast<char*>(buf), len) )
					return len;
			}
		}
		return 0;
	}
};

#endif
