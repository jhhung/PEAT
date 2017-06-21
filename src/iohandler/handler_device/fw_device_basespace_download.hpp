#ifndef FW_DEVICE_BASESPACE_DOWNLOAD_HPP_
#define FW_DEVICE_BASESPACE_DOWNLOAD_HPP_

#include <sstream>
#include <algorithm>// copy, min
#include <iosfwd>
#include <string>// streamsize
#include <cassert>
#include <ios>// ios_base::beg
#include <string>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/positioning.hpp>
//#include "../../thread_pool_update.hpp"
//#include "../../ThreadPool/thread_pool.h"
//#include "../../header/thread_pool.h"
#include "../../thread_control_version.hpp"
#include <mutex>
#include <future>
#include "fw_utility.hpp"
#include "../curl_impl/curl_utility.hpp"
#include "../iohandler.hpp"
#include "device_basespace_setting.hpp"                                                                                                                                                  


template<class SharedMemoryType, class ParaType>
class FDevice<SharedMemoryType, ParaType, FileDeviceType::BasespaceDevice_download>
{
public:
	typedef boost::iostreams::stream_offset size_type;
	typedef boost::iostreams::source_tag category;
	typedef char char_type;

private:
	size_type pos_;
	SharedMemoryType &shared_memory_;
	uint32_t container_size_;
	std::shared_ptr < std::promise <size_t> > sptr_;
	std::shared_ptr< std::future <size_t> > fut_;
	bool eof_;
	int got_length_;

public:
	std::shared_ptr <FWBuffer> bs_buffer_;
	std::vector<int> job_id_;
	CurlImpl <>* curl_device;
	FDevice(SharedMemoryType &sm)
		: pos_(0)
		, shared_memory_(sm)
		, container_size_(0)
		, sptr_(std::make_shared <std::promise <size_t> > ())
		, fut_ (std::make_shared <std::future<size_t> > (sptr_->get_future()))
		, eof_ (false)
		, got_length_ (0)
		, bs_buffer_ (std::make_shared <FWBuffer> ())
		//, curl_device((sm.parameter.bs_curl_config_set_))
	{
		std::cout << "FDevice BasespaceDevice" << std::endl;
		read_impl();
	}

	std::streamsize read(char_type* download_content, std::streamsize length)
	{
		if (bs_buffer_->eof() && eof_)
			return -1;	//return -1 to keep returning -1 for notifying downstream gzip eof situation met

		int active_size=0;
		{
			std::lock_guard <std::mutex> lock (QQQ);
			active_size = bs_buffer_->active_size();
		}

		while (active_size < second_buf_size && !eof_)
		{
			if ( fut_->wait_for (std::chrono::microseconds(100)) == std::future_status::timeout )
			{
				std::chrono::milliseconds dura ( 100 );
				std::this_thread::sleep_for ( dura );
				{   
					std::lock_guard<std::mutex> lock (QQQ);
					active_size = bs_buffer_->active_size();
				}
			}                                                                                                                                                                           
			else if ( fut_->wait_for (std::chrono::microseconds(100)) == std::future_status::ready )
			{   
				for ( auto& Q : job_id_)
					BS_pool.FlushOne(Q);
				eof_ = true;
				break;
			}
		}

		int read_size = 0;
		if (active_size > length)
			read_size = length;
		else if (active_size==0)
		{
			bs_buffer_->setstate(std::ios_base::eofbit);
			//std::cerr<<"bs_buffer_.p & g "<<bs_buffer_->tellg()<<'\t'<<bs_buffer_->tellp()<<'\n';
			return -1;	
		}
		else
			read_size = active_size;

		{
			std::lock_guard<std::mutex> ll(QQQ);
			bs_buffer_ -> read (download_content, read_size);
			got_length_+=read_size;
		}
		return read_size;
	}
	
	~FDevice()
	{
		std::cout << "BasespaceDevice close eof " << shared_memory_.flag_eof_ << std::endl;
		if(shared_memory_.flag_eof_)
			std::cout <<"BaseSpace_download actual close"<<'\n';
		//else
			
	}

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
private:
	//CurlImpl <> curl_device;
    void read_impl (void)
	{
		auto& shared_mem = shared_memory_;	
		auto& sptr2 = *(sptr_.get());	
		auto& bsbuffer2 = *(bs_buffer_.get());
		//curl_device = new CurlImpl <>(shared_mem.parameter.bs_curl_config_set_);
		//auto& cd = curl_device;
		job_id_.push_back ( BS_pool.JobPost ( 
		//[ &shared_mem, &sptr2, &bsbuffer2, &cd]() 
		[ &shared_mem, &sptr2, &bsbuffer2]() 
		{

			uint64_t got_length(0), length_recv(0);
			if (shared_mem.parameter.bs_download_size_!=0)
			{
				while (true)
				{
					got_length += length_recv;
					if (got_length == shared_mem.parameter.bs_download_size_)
						break;
					//cd = new CurlImpl <>(shared_mem.parameter.bs_curl_config_set_);
					CurlImpl <> curl_device (shared_mem.parameter.bs_curl_config_set_);
					curl_device.GetConfig ( (char*) shared_mem.parameter.bs_download_url_.c_str() ); 
					auto ddd = &call_back_operate_mutex<&QQQ, curl_default_handle_mutex>;
					curl_device.WriteConfig (&(bsbuffer2), ddd);
					//cd->GetConfig ( (char*) shared_mem.parameter.bs_download_url_.c_str() );
					//cd->WriteConfig (&(bsbuffer2), &call_back_operate_mutex<(std::mutex*)&QQQ, curl_default_handle_mutex>);
					std::cerr<<"got_length & bs_download_size "<<got_length<<"\t"<<shared_mem.parameter.bs_download_size_<<'\n';
					std::cerr<<"curl_device.url "<<boost::any_cast<char*> (curl_device.config_set_.content_[CURLOPT_URL])<<'\n';
					//std::cerr<<"curl_device.url "<<boost::any_cast<char*> (cd->config_set_.content_[CURLOPT_URL])<<'\n';
	
					std::string temp = ("Range: bytes="+std::to_string(got_length)+"-");
					curl_device.HeaderConfig ( std::vector<std::string>({temp}) );
					//cd->HeaderConfig ( std::vector<std::string>({temp}) );
					curl_device.ExecuteCurl (5566); 
					curl_device.CloseCurl();
					//cd->ExecuteCurl (5566); 
	                //cd->CloseCurl();
					length_recv = curl_device.ReportGetLength();
					auto bs_json_info = curl_device.GetWriteContent().str();	
					//length_recv = cd->ReportGetLength();
                    //auto bs_json_info = cd->GetWriteContent().str();
					std::cerr<<"length_recv & return info: "<<length_recv<<'\t'<<bs_json_info<<'\n';
					//delete cd;
				}
				sptr2.set_value (5566);
				std::cerr<<"DONE CURL GET with total recv length of "<<'\n';
			}
			else
			{
				CurlImpl <> curl_device (shared_mem.parameter.bs_curl_config_set_);
				curl_device.GetConfig ( (char*) shared_mem.parameter.bs_download_url_.c_str() ); 
				auto ddd = &call_back_operate_mutex<&QQQ, curl_default_handle_mutex>;
				curl_device.WriteConfig (&(bsbuffer2), ddd);
				curl_device.ExecuteCurl (5566); 
				//cd = new CurlImpl <>(shared_mem.parameter.bs_curl_config_set_);
				//cd->GetConfig ( (char*) shared_mem.parameter.bs_download_url_.c_str() ); 
                //cd->WriteConfig (&(bsbuffer2), &call_back_operate_mutex<(std::mutex*)&QQQ, curl_default_handle_mutex>);
                //cd->ExecuteCurl (5566);
				sptr2.set_value (5566);
				curl_device.CloseCurl();
				//cd->CloseCurl();
				std::cerr<<"DONE CURL GET with total recv length of "<<'\n';
				//auto bs_json_info = cd->GetWriteContent().str();
				auto bs_json_info = curl_device.GetWriteContent().str();	
				//delete cd;
			}
		}) );
	}
};
#endif
