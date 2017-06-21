#ifndef CURL_IMPL_HPP_
#define CURL_IMPL_HPP_
#include <map>
#include <iostream>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/any.hpp>
#include "curl/curl.h"
#include "curl_utility.hpp"

typedef boost::mpl::vector
<
	char,
	long,
	char*,
	const char*,
	void*,
	curl_slist*,
	FILE*,
	size_t(*)( void* buf, size_t size, size_t nmemb, void* userp )
> 
ARG_TYPE_LIST;

struct SharedContent
{   
	CURLcode code_;
	CURL* curl_;
};

template <typename SHARED_TYPE=SharedContent>
struct CurlSettingDevice
{
    std::pair < const CURLoption, boost::any >& arg_;
	SHARED_TYPE& shared_content_;

	CurlSettingDevice (std::pair <const CURLoption, boost::any >& argin, SHARED_TYPE& shared_obj)
		: arg_ (argin)
		, shared_content_ (shared_obj)
	{}

	template <typename ARG_TYPE>
	void operator() (ARG_TYPE t)
	{
		if (arg_.second.type() == typeid(ARG_TYPE))
		{
			auto result = curl_easy_setopt ( shared_content_.curl_, arg_.first,  boost::any_cast<ARG_TYPE>(arg_.second) );
			if (result == CURLE_OK)
			{
				shared_content_.code_ = CURLE_OK;
				std::cerr<<"setting ok : "<<arg_.first<<'\n';
			}
			else
				std::cerr<<"setting failed : "<<arg_.first<<'\n';
		}
	}
};

enum CurlConfigType
{
	bs_config
};

template <int CURLCONFIG = CurlConfigType::bs_config>
struct CurlConfigSet
{
	std::map <CURLoption, boost::any> content_;

	CurlConfigSet (void)
		: content_ ({{CURLOPT_VERBOSE, 1L},{CURLOPT_NOSIGNAL, 1L},{CURLOPT_SSL_VERIFYHOST, 0L},{CURLOPT_SSL_VERIFYPEER, 0L}})
	{}	//SSL verify host & peer set to 1L for TLS error; may need to set back to 0L on Amazon

	CurlConfigSet (std::map<CURLoption, boost::any>&in)
		: content_ (in)
	{}

	CurlConfigSet (std::map<CURLoption, boost::any>&&in)
		: content_ (in)
	{}

	void reset (void)
	{
	//	std::map <CURLoption, boost::any> temp ({{CURLOPT_VERBOSE, 1L},{CURLOPT_NOSIGNAL, 1L},{CURLOPT_SSL_VERIFYHOST, 0L},{CURLOPT_SSL_VERIFYPEER, 0L}});
		CurlConfigSet QQ;
		content_.swap(QQ.content_);
	}
};


template <typename INTYPE=CurlConfigSet<>, typename SHARED_TYPE=SharedContent>
struct CurlImpl
{
	SHARED_TYPE shared_obj_;
	INTYPE config_set_;
	int error_count_;
	int error_tolerance_;
	std::stringstream default_buf_;

	CurlImpl (const CurlImpl& in)
		: shared_obj_(in.shared_obj_)
		, config_set_(in.config_set_)
		, error_count_(in.error_count_)
		, error_tolerance_ (in.error_tolerance_)
	{
	    curl_global_init (CURL_GLOBAL_ALL);
	}

	CurlImpl (int error_tolerance = 5)
		: shared_obj_()
		, config_set_()
		, error_count_(0)
		, error_tolerance_ (error_tolerance)
	{
	    curl_global_init (CURL_GLOBAL_ALL);
	}

	CurlImpl (INTYPE& parameter_in, int error_tolerance = 5)
		: shared_obj_()
		, config_set_ (parameter_in)
		, error_count_(0)
		, error_tolerance_ (error_tolerance)
	{
	    curl_global_init (CURL_GLOBAL_ALL);
	}

	CURLcode ExecuteCurl (int ii=0)//(void)
	{
  		shared_obj_.code_ = CURLE_FAILED_INIT;
	    shared_obj_.curl_ = curl_easy_init();

//std::cerr<<"@@@EXECUTE: current post url  in & real "<<boost::any_cast<char*>(config_set_.content_[CURLOPT_URL])<<'\n';
		for (auto&item : config_set_.content_)
			boost::mpl::for_each <ARG_TYPE_LIST> (CurlSettingDevice<> (item, shared_obj_));

		return ExecuteCurlImpl (ii);
	}
	
	CURLcode ExecuteCurlImpl (int ii=0)//(void)
	{
//	std::cerr<<"####AAAAA: current Execute url : "<<boost::any_cast<char*>(config_set_.content_[CURLOPT_URL])<<'\n';
		try
		{
			if (shared_obj_.code_ == CURLE_OK)
				shared_obj_.code_ = curl_easy_perform (shared_obj_.curl_);
			else
				throw curl_send_exception_object;
//			std::cerr<<"####curl execute perform: "<<curl_easy_strerror(shared_obj_.code_)<<std::endl;
//			std::cerr<<"####current Execute url : "<<boost::any_cast<char*>(config_set_.content_[CURLOPT_URL])<<'\n';

			std::stringstream ss;
			ss << curl_easy_strerror(shared_obj_.code_);

			if (ss.str().find ("No error") == std::string::npos)
			{
				if (ii==0)
					throw curl_send_exception_object;
				else
					throw curl_recv_exception_object;
			}
			else
				;
			return shared_obj_.code_;
		}
		catch (curl_send_exception& excp)//(std::exception& excp)
		{
			std::cerr<<"#### curl send exception catched "<<excp.what()<<std::endl;
			std::cerr<<"#### repeated times: "<<error_count_<<std::endl;
			if (error_count_ < error_tolerance_)
			{
				++error_count_;
				std::chrono::milliseconds dura( 3000 );
				std::this_thread::sleep_for( dura );
				shared_obj_.code_ = CURLE_OK;
				ExecuteCurlImpl ();
			}
			else
			{
				std::cerr<<"#### Exit for repeat error "<<std::endl;
				//exit(1);
                return shared_obj_.code_;
			}
		}

		catch (curl_recv_exception& excp)//(std::exception& excp)
		{
			std::cerr<<"#### curl recv exception catched "<<excp.what()<<std::endl;
			std::cerr<<"#### Exit for repeat error "<<std::endl;
			//exit(1);
			return shared_obj_.code_;
		}

	}
/*
	uint64_t ExecuteCurl (int overload_dummy)
	{
  		shared_obj_.code_ = CURLE_FAILED_INIT;
	    shared_obj_.curl_ = curl_easy_init();

//std::cerr<<"@@@EXECUTE: current post url  in & real "<<boost::any_cast<char*>(config_set_.content_[CURLOPT_URL])<<'\n';
		for (auto&item : config_set_.content_)
			boost::mpl::for_each <ARG_TYPE_LIST> (CurlSettingDevice<> (item, shared_obj_));

		return ExecuteCurlImpl (overload_dummy);
	}

	uint64_t ExecuteCurlImpl (int overload_dummy)
	{
		try
		{
			if (shared_obj_.code_ == CURLE_OK)
				shared_obj_.code_ = curl_easy_perform (shared_obj_.curl_);
			else
				throw curl_send_exception_object;

			std::stringstream ss;
			ss << curl_easy_strerror(shared_obj_.code_);

			if (ss.str().find ("No error") == std::string::npos)
				throw curl_send_exception_object;
			else
				;
			//!!!!!!!!!!!!!!!!!adding get inpart function!!!!!!!!!!!!!!!!!!!
			double str;
			shared_obj_.code_ = curl_easy_getinfo ( shared_obj_.curl_, CURLINFO_SIZE_DOWNLOAD, &str );
			uint64_t length_r = (uint64_t) str;
			std::cerr<<"curl recv length : return code "<<length_r<<'\t'<<curl_easy_strerror(shared_obj_.code_)<<std::endl;
			return length_r;//shared_obj_.code_;
		}
		catch (std::exception& excp)
		{
			std::cerr<<"#### curl exception catched "<<excp.what()<<std::endl;
			std::cerr<<"#### repeated times: "<<error_count_<<std::endl;
			if (error_count_ < error_tolerance_)
			{
				++error_count_;
				std::chrono::milliseconds dura( 3000 );
				std::this_thread::sleep_for( dura );
				shared_obj_.code_ = CURLE_OK;
				ExecuteCurlImpl (5566);
			}
			else
			{
				std::cerr<<"#### Exit for repeat error "<<std::endl;
				exit(1);
			}
		}
	}
*/
	void CloseCurl (void)
	{
		curl_easy_cleanup (shared_obj_.curl_);
		error_count_ = 0;
		config_set_.reset();
	}

	uint64_t ReportGetLength (void)
	{
        double str;
        shared_obj_.code_ = curl_easy_getinfo ( shared_obj_.curl_, CURLINFO_SIZE_DOWNLOAD, &str );
        uint64_t length_r = (uint64_t) str;
        std::cerr<<"curl recv before return "<<curl_easy_strerror(shared_obj_.code_)<<std::endl;
        return length_r;
	}

	void PostConfig (char* url, char* post_field="", int post_length=0)
	{
		config_set_.content_[CURLOPT_URL] = url;
		config_set_.content_[CURLOPT_POST] = 1L;
		config_set_.content_[CURLOPT_POSTFIELDSIZE] = (long) post_length;//strlen(post_field);
		config_set_.content_[CURLOPT_POSTFIELDS] =  post_field;
		config_set_.content_[CURLOPT_WRITEDATA] = (void*)&default_buf_;
		config_set_.content_[CURLOPT_WRITEFUNCTION] = &call_back_operate_mutexless <curl_default_handle>;
//std::cerr<<"current post url  in & real "<<url<<'\t'<<boost::any_cast<char*>(config_set_.content_[CURLOPT_URL])<<'\n';
	}

	void PutConfig (char* url, char* post_field, int post_length)
	{
		config_set_.content_[CURLOPT_URL] = url;
		config_set_.content_[CURLOPT_CUSTOMREQUEST] = (char*)("PUT");
		config_set_.content_[CURLOPT_POSTFIELDSIZE] = (long) post_length;//strlen(post_field);
		config_set_.content_[CURLOPT_POSTFIELDS] = post_field;
		config_set_.content_[CURLOPT_WRITEDATA] =  (void*)&(default_buf_);
		config_set_.content_[CURLOPT_WRITEFUNCTION] = &call_back_operate_mutexless <curl_default_handle>;
	}

	void GetConfig (char* url)
	{
		config_set_.content_[CURLOPT_URL] = url;
		config_set_.content_[CURLOPT_WRITEDATA] = (void*)&default_buf_;
		config_set_.content_[CURLOPT_WRITEFUNCTION] = &call_back_operate_mutexless <curl_default_handle>;
		config_set_.content_[CURLOPT_NOPROGRESS] = 1L;
		config_set_.content_[CURLOPT_FOLLOWLOCATION] = 1L;
	}

	void HeaderConfig (const std::vector<std::string>& header_content)
	{
		curl_slist * pHeaders = NULL;
		for (auto& item : header_content)
			pHeaders = curl_slist_append (pHeaders, item.c_str());
		config_set_.content_[CURLOPT_HTTPHEADER] = pHeaders;
	} 

	void WriteConfig (void* write_field, size_t(*callback)(void* buf, size_t size, size_t nmemb, void* userp) )
	{
		config_set_.content_[CURLOPT_WRITEDATA] = write_field;
		config_set_.content_[CURLOPT_WRITEFUNCTION] = callback;
	}

	void ChangeRecvBuf (void* write_field, size_t(*callback)(void* buf, size_t size, size_t nmemb, void* userp) )
	{
		WriteConfig (write_field, callback);
	}

	std::stringstream& GetWriteContent (void)
	{
		return default_buf_;
	}

	void CustomConfig (std::map <CURLoption, boost::any>& custom_set)
	{
		config_set_.content_ = custom_set;
	}
};
#endif
