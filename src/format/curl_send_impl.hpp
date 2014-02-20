/**
 *  @file curl_send.hpp
 *  @brief provide basic class for curl sending detail 
 *  @author C-Salt Corp.
 */
#ifndef CURL_SEND_IMPL_HPP_
#define CURL_SEND_IMPL_HPP_
#include "curl_recv_impl.hpp"
#include "json_handler.hpp"

/**
 * @class curl_send_exception
 * @brief define an exception class, public inherited from std::exception and thrown in send function
 */
class curl_send_exception: public std::exception
{
    virtual const char* what() const throw()
    {
        return "curl_send exception happened";
    }
}
curl_send_exception_object;


/** 
 * @class CurlSendImpl
 * @brief provide curl data sending implementation. 
 * @tparam M a pointer to std::mutex object, served as a non-type parameter for passing in the std::mutex object competed by threading functions of call_back_operate_mutex functor and parsing function of CurlWrapper class.  
 * @tparam FUNC a type parameter indicating the type of the call_back functor called withing the global call_back_operate_mutex function, defaulted as the above mentioned curl_default_handle_mutex
 */
template <typename FUNC=curl_default_handle>
struct CurlSendImpl
{
	std::string url_upload_json_, token_, content_type_, default_str_;
	size_t part_index_;
	std::string json_recv_1_,url_upload_content_;
	boost::property_tree::ptree pt_;
	uint64_t sent_size_;

	CurlSendImpl& operator= (const CurlSendImpl& in)
	{
		url_upload_json_ = (in.url_upload_json_);
		token_ = (in.token_);
		content_type_ = (in.content_type_);
		default_str_ = (in.default_str_);
		part_index_ = (0);
		send_1 ();
		parse_json_1 ();
		sent_size_ = in.sent_size_;
	}

	CurlSendImpl (void)
		: url_upload_json_ ("")
		, token_ ("")
		, content_type_ ("")
		, default_str_ ("")
		, part_index_ (0)
		, sent_size_ (0)
	{}

	CurlSendImpl (const CurlSendImpl& in)
		: url_upload_json_ (in.url_upload_json_)
		, token_ (in.token_)
		, content_type_ (in.content_type_)//("Content-Type: application/binary")//(content_type)
		, default_str_ (in.default_str_)
		, part_index_ (0)
		, json_recv_1_ (in.json_recv_1_)
		, sent_size_ (0)
	{
		parse_json_1 ();
	}

	CurlSendImpl (const std::string& url, const std::string& token, const std::string& content_type, const std::string& default_str)
		: url_upload_json_ (url)
		, token_ (token)
		, content_type_ (content_type)//("Content-Type: application/binary")//(content_type)
		, default_str_ (default_str)
		, part_index_ (0)
		, sent_size_ (0)
	{
		send_1 ();
		parse_json_1 ();
	}

	CURLcode send_1 (void)
	{
		std::stringstream ss;
		CURLcode code (CURLE_FAILED_INIT);
		CURL* curl = curl_easy_init();
		curl_slist * pHeaders = NULL;
		pHeaders = curl_slist_append (pHeaders, token_.c_str());//cmd);
		pHeaders = curl_slist_append (pHeaders, content_type_.c_str());//cmd2);

        try
        {
        if (curl)
        {
            if (   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, url_upload_json_.c_str() ) ) 
                && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POST, 1L) )
                && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1L) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, 0) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, NULL) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, call_back_operate_mutexless <FUNC> ) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(ss) ) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) ) 
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOSIGNAL, 1) ) //for threading purpose 
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
                )
            {
                    code = curl_easy_perform (curl);
                    std::cerr<<"curl send_1 easy perform "<<curl_easy_strerror(code)<<std::endl;
                    if (code != CURLE_OK)
                        throw curl_recv_exception_object;
            }
            else
            {
                std::cerr<<"CURLcode curl send_1 not CURLE_OK "<<curl_easy_strerror(code)<<std::endl;
                throw curl_send_exception_object;
            }
            curl_easy_cleanup (curl);
        }
        else                                                                                                                                                      
        {                                                                                                                                                             
            std::cerr<<"curl init exception CURLcode "<<curl_easy_strerror(code)<<std::endl;
            throw curl_send_exception_object;
        }
        json_recv_1_ = ss.str();
        std::cerr<<"oooo"<<json_recv_1_<<json_recv_1_<<'\n';
        return code;
        }
        catch (std::exception& excp)
        {
            std::cerr<<"curl_send_1 exception catched "<<excp.what()<<std::endl;
code = curl_easy_perform (curl);
return code;
		}
	}

	void parse_json_1 (void)//(const std::string& default_str)
	{
		std::stringstream ss (json_recv_1_);
		boost::property_tree::json_parser::read_json ( ss, pt_ );
		boost::property_tree::ptree& pos = pt_.get_child ("Response");
		content_type_ = ("ContentType: ") + pos.get <std::string> ("ContentType");
		auto temp = pos.get <std::string> ("HrefContent");
		temp.resize (temp.rfind ("/content"));
		url_upload_content_ = default_str_ + temp;
	}

	void send (char* ssin, bool flag, int lengthin )
	{
			auto url = update_url_upload ();//(flag);
			send_2 (ssin, url, lengthin);
	}
	
	std::string update_url_upload (void)//(bool flag)
	{
		++part_index_;
		boost::property_tree::ptree& pos = pt_.get_child ("Response");
		{
			std::stringstream st;
			st << part_index_;
			auto temp = url_upload_content_ + ("/parts/") + st.str();//(char*)part_index_;
			return temp;
		}
	}

	CURLcode send_2 (char* upload_content, std::string& url, size_t lengthin)
	{
		CURLcode code (CURLE_FAILED_INIT);
		CURL* curl = curl_easy_init();
		curl_slist * pHeaders = NULL;
		pHeaders = curl_slist_append (pHeaders, token_.c_str());
		pHeaders = curl_slist_append (pHeaders, content_type_.c_str());
		pHeaders = curl_slist_append (pHeaders, "Content-Type: multipart/form-data");	//needed only in gzupload.  will cause txt transmission failed.
		std::stringstream sq;

        try 
        {   
            if (curl)
            {
                if (   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, url.c_str() ) ) 
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) ) 
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_CUSTOMREQUEST, "PUT") ) /* !!! */
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, upload_content ) )//(void*) gg.c_str() ) ) /* data goes here */
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, lengthin ) )//gg.size()) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, call_back_operate_mutexless <FUNC> ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(sq) ) )

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOSIGNAL, 1) ) //for threading purpose 

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
                        )
                        code = curl_easy_perform (curl);
                else
                {
                    std::cerr<<"curl send 2 not CULRE_OK "<<curl_easy_strerror(code)<<std::endl;
                    throw curl_send_exception_object;
                }
                curl_easy_cleanup (curl);
            }
            else
            {
                std::cerr<<"curl send 2 init exception "<<curl_easy_strerror(code)<<std::endl;
                throw curl_send_exception_object;
            }
            std::cerr<<"response json"<<sq.str()<<'\n';
            if (sq.str().find ("ErrorCode") != std::string::npos)
                throw curl_send_exception_object;
        }
        catch (std::exception& excp)
        {
            std::cerr<<"curl_send_2 exception catched "<<excp.what()<<std::endl;
//          curl_easy_cleanup (curl);
            return code;
        }
        return code;
	}

	CURLcode send_end (void)
	{
		std::string url = url_upload_content_+ ("?uploadstatus=complete");
		CURLcode code (CURLE_FAILED_INIT);
		CURL* curl = curl_easy_init();
		curl_slist * pHeaders = NULL;
		pHeaders = curl_slist_append (pHeaders, token_.c_str());
		pHeaders = curl_slist_append (pHeaders, content_type_.c_str());
		std::stringstream sq;

        try
        {
            if (curl)
            {
                if (   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, url.c_str() ) ) 
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POST, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, "") ) /* data goes here */
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) ) 
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, call_back_operate_mutexless <FUNC> ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(sq) ) )

&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOSIGNAL, 1) ) //for threading purpose 

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
                   )
                    code = curl_easy_perform (curl);
                else
                {
                    std::cerr<<" send end CURLE_OK not ok "<<curl_easy_strerror(code)<<std::endl;
                    throw curl_send_exception_object;
                }
                curl_easy_cleanup (curl);
            }
            else
            {
                std::cerr<<"CURLcode init exception "<<curl_easy_strerror(code)<<std::endl;
                throw curl_send_exception_object;
            }
            std::cerr<<"response json"<<sq.str()<<'\n';
            if (sq.str().find ("ErrorCode") != std::string::npos)
                throw curl_send_exception_object;
        }
        catch (std::exception& excp)
        {
            std::cerr<<"curl_send_end exception catched "<<excp.what()<<std::endl;
curl_easy_cleanup (curl);
return code;
        }
        return code;
	}

	CURLcode send_progress_info (const std::string& urlin, size_t uid_in, size_t curl_in_ratio, bool end=false)
	{
		std::stringstream ss;
		CURLcode code (CURLE_FAILED_INIT);
		CURL* curl = curl_easy_init();
		curl_slist * pHeaders = NULL;

        if (!end)
        	ss << (urlin + token_.substr (16) + '?') << "files_uid=" << uid_in << "&f1_Status=" << curl_in_ratio;
        else
	        ss << (urlin + token_.substr (16) + '?') << "files_uid=" << uid_in << "&f1_Status=" << 10000;
        std::string response_url (ss.str());
		std::cerr<<"send_progress_info "<<response_url<<'\t'<<curl_in_ratio<<std::endl;
        ss.str("");
        try
        {
            if (curl)
            {
                if (   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, response_url.c_str() ) ) 
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POST, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, 0) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, NULL) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, call_back_operate_mutexless <FUNC> ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(ss) ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) ) 

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOSIGNAL, 1) ) //for threading purpose 

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
                   )
                    code = curl_easy_perform (curl);
                else
                {
                    std::cerr<<"send progress_info code not CURLE_OK "<<curl_easy_strerror(code)<<std::endl;
                    throw curl_send_exception_object;
                }
                curl_easy_cleanup (curl);
            }
            else
            {
                std::cerr<<"send progress info  init exception "<<curl_easy_strerror(code)<<std::endl;
                throw curl_send_exception_object;
            }
        }
        catch (std::exception& excp)
        {
            std::cerr<<"curl_send_progress_info exception catched "<<excp.what()<<std::endl;
curl_easy_cleanup (curl);
return code;
        }
		json_recv_1_ = ss.str();
		std::cerr<<"send_progress_info"<<json_recv_1_<<'\n';//json_recv_1_<<'\n';
		return code;
	}

	CURLcode send_response (const std::string& urlin)
	{
		std::stringstream ss;
		CURLcode code (CURLE_FAILED_INIT);
		CURL* curl = curl_easy_init();
		curl_slist * pHeaders = NULL;
		std::string response_url ( urlin + token_.substr ( 16 ) );
        try
        {
            if (curl)
            {
                if (   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, response_url.c_str() ) ) 
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POST, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1L) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, 0) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, NULL) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, call_back_operate_mutexless <FUNC> ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(ss) ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) ) 

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_NOSIGNAL, 1) ) //for threading purpose 

                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
                        && CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
                   )
                    code = curl_easy_perform (curl);
                else
                {
                    std::cerr<<"curl_send_response code not CURLE_OK  "<<curl_easy_strerror(code)<<std::endl;
                    throw curl_send_exception_object;
                }
                curl_easy_cleanup (curl);
            }
            else
            {
                std::cerr<<"send response init exception "<<curl_easy_strerror(code)<<std::endl;
                throw curl_send_exception_object;
            }
        }
        catch (std::exception& excp)
        {
            std::cerr<<"curl_send_progress_info exception catched "<<excp.what()<<std::endl;
            curl_easy_cleanup (curl);
            return code;
        }
		json_recv_1_ = ss.str();
		std::cerr<<"termination info"<<json_recv_1_<<'\n';//json_recv_1_<<'\n';
		return code;
	}
}; 

#endif
