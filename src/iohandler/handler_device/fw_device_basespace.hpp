#pragma once
#include <algorithm>// copy, min
#include <cassert>
#include <iosfwd>
#include <ios>// ios_base::beg
#include <mutex>
#include <future>
#include <string>// streamsize
#include <sstream>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
//#include "../../thread_pool_update.hpp"
//#include "../../ThreadPool/thread_pool.h"
//#include "../../header/thread_pool.h"
#include <thread_control_version.hpp>
#include <iohandler/handler_device/fw_utility.hpp>
#include <iohandler/curl_impl/curl_utility.hpp>
// #include "PEAT/iohandler.hpp"
#include <iohandler/iohandler.hpp>
#include <iohandler/handler_device/device_basespace_setting.hpp>

// namespace peat{
template<class SharedMemoryType, class ParaType>
class FDevice<SharedMemoryType, ParaType, FileDeviceType::BasespaceDevice>
{
private:
	typedef boost::iostreams::stream_offset size_type;
	size_type pos_;
	SharedMemoryType &shared_memory_;
	uint32_t container_size_;
	int part_index_;

public:
	std::shared_ptr <FWBuffer> bs_buffer_;
	std::shared_ptr <std::string> upload_url_;
	std::vector<size_t> job_id_;
	size_t open_job_id_;
	bool is_closed_;

	//typedef boost::iostreams::seekable_device_tag category;
	typedef boost::iostreams::sink_tag  category;
	typedef char char_type;
	typedef typename at<ParaType, typename BaseSpaceLocalParameter::CurlSendMaxLength, boost::mpl::int_<24*1024*1024> >::type CurlSendMaxLengthType;
	typedef typename at<ParaType, typename BaseSpaceLocalParameter::CurlSendLastLength, boost::mpl::int_<5*1024*1024> >::type CurlSendLastLengthType;

	FDevice(SharedMemoryType &sm)
		: pos_(0)
		, shared_memory_(sm)
		, container_size_(0)
		, part_index_(0)
		, bs_buffer_ (std::make_shared <FWBuffer> ())
		, upload_url_ (std::make_shared <std::string> ())
		, is_closed_(false)
	{
		std::cout << "FDevice BasespaceDevice" << std::endl;
		open();
	}

	void open()
	{
		auto& shared_mem = shared_memory_;  
		auto& upload_url = *(upload_url_.get());
        open_job_id_ = BS_pool.JobPost (
		[this, &shared_mem, &upload_url]
		{
			CurlImpl <> curl_device (shared_mem.parameter.bs_curl_config_set_);
			curl_device.PostConfig ((char*) shared_mem.parameter.bs_file_open_url_.c_str());//(curl_url);	
			curl_device.HeaderConfig (std::vector<std::string>(
				{shared_mem.parameter.bs_access_token_, shared_mem.parameter.bs_content_type_}));
			curl_device.ExecuteCurl (); 
			curl_device.CloseCurl();
			//std::cerr<<"open return json "<< curl_device.GetWriteContent().str()<<'\n';
			upload_url = parse_upload_url(curl_device.GetWriteContent().str(), shared_mem );
			//std::cerr<<"upload url "<<upload_url<<'\n';//shared_mem.parameter.bs_upload_url_<<'\n';
		});
		//std::cerr<<"open_job_id "<<open_job_id_<<'\n';
	}

	std::streamsize write(const char_type* upload_content, std::streamsize length)
	{
		auto active_size = bs_buffer_->active_size();
		BS_pool.FlushOne (open_job_id_);
		//std::cerr<<"write upload_url "<<upload_url_.get()<<'\t'<<*upload_url_<<'\n';
		if (active_size>CurlSendMaxLengthType::value)
			write_impl (active_size, update_url_upload ());
		else
			;
		bs_buffer_->write (upload_content, length);
		return length;
	}

	std::streamsize read(const char_type* download_content, std::streamsize length)
	{}

	boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
	{
		boost::iostreams::stream_offset next;
		if (way == std::ios_base::beg) 
		{
			next = off;
		} 
		else if (way == std::ios_base::cur) 
		{
			next = pos_ + off;
		} 
		else if (way == std::ios_base::end) 
		{
			next = container_size_ + off - 1;
		} 
		else 
		{
			throw std::ios_base::failure("bad seek direction");
		}
		// Check for errors
		if (next < 0 || next >= container_size_)
			throw std::ios_base::failure("bad seek offset");

		pos_ = next;
		return pos_;
	}
	
	~FDevice()
	{
		std::cout << "BasespaceDevice close eof " << shared_memory_.flag_eof_ << std::endl;
		if(shared_memory_.flag_eof_)
		{
			if (is_closed_)
				;
			else
				close();

			for (auto& yy : job_id_)
				BS_pool.FlushOne (yy);
		}
	}

	void close()
	{
		is_closed_=true;

        auto active_size = bs_buffer_->active_size();
		std::vector<int> send_length;// remain_length=0, last_length=0;
        if (active_size > CurlSendLastLengthType::value)//5*1024*1024)	
        {
        	auto n_max = std::max(active_size - CurlSendLastLengthType::value, CurlSendLastLengthType::value);
        	auto n_min = std::min(active_size - CurlSendLastLengthType::value, CurlSendLastLengthType::value);
			std::vector<int> temp ({n_max, n_min});//5*1024*1024), 5*1024*1024});
			send_length.swap (temp);
		}
		else
			send_length.push_back (active_size);

		BS_pool.FlushOne (open_job_id_);

		for (auto& item: send_length)
			write_impl (item, update_url_upload ());
		
		auto& upload_url = *(upload_url_.get());
		auto temp_job_id = job_id_;

		job_id_.push_back( BS_pool.JobPost (
		[this, &upload_url] 
		{
			CurlImpl <> curl_device (shared_memory_.parameter.bs_curl_config_set_);
			//std::cout << "BasespaceDevice real close " << shared_memory_.flag_eof_ << std::endl;
			//std::cerr<<"threading upload_url_"<<&upload_url<<'\t'<<upload_url_.get()<<'\t'<<*(upload_url_.get())<<'\t'<<upload_url<<'\n';
			auto curlstr = std::string (upload_url + ("?uploadstatus=complete"));
			char* curl_url = (char*) curlstr.c_str();
			//std::cerr<<"close: adress of shared_memory_.parameter "<<&(shared_memory_.parameter)<<"url "<<curlstr<<'\n';
			curl_device.PostConfig ((char*)curlstr.c_str());//(curl_url);//(char*)yy.c_str());//((char*) std::string (bs_upload_url_+ ("?uploadstatus=complete")).c_str() );
			curl_device.HeaderConfig (std::vector<std::string>({shared_memory_.parameter.bs_access_token_, shared_memory_.parameter.bs_content_type_}));
			//std::cerr<<"complete url origin: in : out "<<curlstr<<'\t'<<boost::any_cast<char*>(curl_device.config_set_.content_[CURLOPT_URL])<<'\n';
			curl_device.ExecuteCurl (); 
			curl_device.CloseCurl();

			//std::string response_url ( shared_memory_.parameter.bs_finish_url_ + shared_memory_.parameter.bs_access_token_.substr ( 16 ) );	
			//curl_device.PostConfig ((char*) response_url.c_str());
			//curl_device.ExecuteCurl (); 
			//curl_device.CloseCurl();
		},
		temp_job_id ) );
	}

private:
	std::string parse_upload_url(const std::string&& bs_json_put_content, SharedMemoryType& shared_memory)
	{
		std::string upload_url;
		std::stringstream ss (bs_json_put_content);
		boost::property_tree::ptree bs_pt_;
		boost::property_tree::json_parser::read_json ( ss, bs_pt_ );
		boost::property_tree::ptree& pos = bs_pt_.get_child ("Response");
		shared_memory.parameter.bs_content_type_ = ("ContentType: ") + pos.get <std::string> ("ContentType");
		auto temp = pos.get <std::string> ("HrefContent");
		temp.resize (temp.rfind ("/content"));
		upload_url = shared_memory.parameter.bs_basic_url_ + temp;
		//std::cerr<<"Oo "<<upload_url<<'\n';//shared_memory.parameter.bs_upload_url_<<'\n';
		return upload_url;//shared_memory.parameter.bs_upload_url_;//upload_url;
		//std::cerr<<"bs_upload_url_ "<<bs_upload_url_<<'\n';
	}
	
	std::string update_url_upload (void)
	{   
		++part_index_;
		//std::cerr<<"%%%% inpart url "<<shared_memory_.parameter.bs_upload_url_ + ("/parts/") + std::to_string (part_index_)<<'\n';
		return *upload_url_ + ("/parts/") + std::to_string (part_index_);//st.str();
	}

	void write_impl (int active_size, std::string&& upload_inpart_url)
	{
		//std::cerr<<"write_impl upload_url "<<upload_url_.get()<<'\t'<<*upload_url_<<'\n';
		char* ka = new char [active_size+1];
		bs_buffer_->read (ka, active_size);
		if(bs_buffer_->eof())
		{
			bs_buffer_->clear();
			bs_buffer_->seekp(0);
			bs_buffer_->seekg(0);
		}
		job_id_.push_back ( BS_pool.JobPost (
		[ka, this, upload_inpart_url, active_size] 
		{
        	for (auto ind=0; ind!=5; ++ind)
        	{
        	    CurlImpl <> curl_device (shared_memory_.parameter.bs_curl_config_set_);
        	    curl_device.HeaderConfig (std::vector<std::string>({
        	        shared_memory_.parameter.bs_access_token_,
        	        shared_memory_.parameter.bs_content_type_,
        	        shared_memory_.parameter.bs_upload_content_type_}));//  std::string("Content-Type: multipart/form-data")}));
        	    //char* curl_url = (char*) upload_inpart_url.c_str();
        	    //std::cerr<<"write_impl url "<<yy<<'\n';
        	    curl_device.PutConfig ( (char*) upload_inpart_url.c_str() , ka, active_size);//(char*)//upload_content);
        	    auto code = curl_device.ExecuteCurl (); 
        	    curl_device.CloseCurl();
        	    auto bs_json_info = curl_device.GetWriteContent().str();

        	    std::stringstream ss;
        	    ss << curl_easy_strerror(code);

        	    if (ss.str().find ("No error") == std::string::npos)
        	        continue;
        	    else                                                                                                                                                                         
        	        break;
        	}  
			delete ka;	
		}
		, std::vector<size_t>({open_job_id_}) ) );
	}
};

// }
