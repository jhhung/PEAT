#ifndef IO_HANDLER_HPP_
#define IO_HANDLER_HPP_

#include <iostream>
#include <string>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>
#include <deque>
#include <functional>
#include <fstream>
#include <boost/type_traits.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/mpl/char.hpp>
#include <boost/mpl/string.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/back.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <memory>
#include "analyzer_utility.hpp"
#include "curl_impl/curl_impl.hpp"
#include "../constant_def.hpp"

template<class SharedMemoryType, class ParaType, int FileDeviceType>
class FDevice
{};

#include "handler_device/fw_device_basespace.hpp"
#include "handler_device/fw_device_basespace_download.hpp"
#include "handler_device/fw_device_container.hpp"
#include "handler_device/fw_device_ofstream.hpp"
#include "handler_device/fw_device_ifstream.hpp"
#include "handler_device/fw_device_stream.hpp"
#include "handler_device/fw_filter_gzip_compressor.hpp"
#include "handler_device/fw_filter_gzip_decompressor.hpp"


struct IoHandlerGlobalParameter
{
	typedef boost::mpl::int_<0> FilteringStreamType;
	typedef boost::mpl::int_<1> DeviceParameter;
	typedef boost::mpl::int_<2> DeviceType;
	typedef boost::mpl::int_<3> MutipleNumber;
	typedef boost::mpl::int_<4> DeviceBufferSize;
	typedef boost::mpl::int_<5> DevicePushbackSize;
	typedef boost::mpl::int_<6> BaseStreamType;
};

struct DeviceParameter
{
	/// @brief general
	std::string directory;
	std::string filename;
	std::string in_filename;
	
	/// @basesapce
	std::string bs_basic_url_;
	std::string bs_version_;
	std::string bs_app_result_;
	std::string bs_file_path_;
	std::string bs_dir_path_;
	std::string bs_tailing_str_;
	std::string bs_file_open_url_;//determining the json url for BaseSpace communication, e.g. https://api.basespace.illumina.com/v1pre3/
	std::string bs_access_token_;
	std::string bs_content_type_;
	std::string bs_upload_content_type_;
	std::string bs_upload_url_;

	std::string bs_download_url_;
	uint64_t bs_download_size_;

	std::string bs_finish_url_;
	std::string bs_progress_url_;
	CurlConfigSet<CurlConfigType::bs_config> bs_curl_config_set_;
};


template<class SETTING>
struct SharedMemory
{
	typedef typename at<SETTING, typename IoHandlerGlobalParameter::MutipleNumber, boost::mpl::int_<2> >::type MutipleNumberType;
	typedef typename at<SETTING, typename IoHandlerGlobalParameter::DeviceParameter, DeviceParameter >::type DeviceParameterType;
	
	DeviceParameterType parameter;
	
	bool flag_eof_;
	std::mutex sm_mutex;
	
	SharedMemory(int pipeline_size)
		:flag_eof_(false)
	{}
};

template<class FilteringStreamType, class SharedMemoryType>
class PushDevice
{
	FilteringStreamType &filter_stream;
	SharedMemoryType &shared_memory_;
	void* transfer_ptr_;
public:
	PushDevice(FilteringStreamType &fs, SharedMemoryType &sm)
		: shared_memory_(sm), filter_stream(fs)
	{}
	template<class FWObjPara>
	void operator()(FWObjPara type)
	{
	
		typedef typename boost::mpl::at<FWObjPara, typename IoHandlerGlobalParameter::DeviceType>::type DeviceType;
		FDevice<SharedMemoryType, FWObjPara, DeviceType::value> device(shared_memory_);
		
		filter_stream.push(device);//, 2621400);
	}
};


template <class LIST, int N, int EM=999>
struct Closer
{
	typedef typename boost::mpl::at< LIST, boost::mpl::int_<N-1> >::type map_type;
	typedef typename boost::mpl::at <map_type, IoHandlerGlobalParameter::DeviceType>::type d_type;
	typedef typename Closer < LIST, N-1, d_type::value >::type type;

	Closer (int i)                                                                                                                                                                      
	{  
	    std::cerr<<"current N & EM "<<N<<'\t'<<EM<<'\n';
	    std::cerr<<"map_type[device_type] "<<d_type::value<<'\n';
	    std::cerr<<"type "<<typeid(type).name()<<'\n';
	    Closer < LIST, N-1, d_type::value > kk(1);
	}   
};

template <class LIST, int N>
struct Closer <LIST, N, FileDeviceType::GzipCompressFilter> 
{
	typedef typename boost::mpl::at< LIST, boost::mpl::int_<N> >::type type;

	Closer (int i)                                                                                                                                                                      
	{  
	    std::cerr<<"current N & EM "<<N<<'\t'<<"gz"<<'\n';
	    std::cerr<<"type "<<typeid(type).name()<<'\n';
	}   
};

template <class LIST, int N>
struct Closer <LIST, N, FileDeviceType::GzipDeCompressFilter> 
{
	typedef typename boost::mpl::at< LIST, boost::mpl::int_<N> >::type type;

	Closer (int i)                                                                                                                                                                      
	{  
	    std::cerr<<"current N & EM "<<N<<'\t'<<"degz"<<'\n';
	    std::cerr<<"type "<<typeid(type).name()<<'\n';
	}   
};

//template <class LIST, int EM>
//struct Closer <LIST, 0, EM>
//{
//	typedef boost::mpl::map <boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::GzipCompressFilter> > > type;
//};


template<class FilteringStreamType, class SharedMemoryType, class LIST>
class CloseDevice
{
	FilteringStreamType &filter_stream_;
	SharedMemoryType &shared_memory_;
	int index;
	bool is_gz_device_;
	public:
	CloseDevice(FilteringStreamType &fs, SharedMemoryType &sm)
		: shared_memory_(sm), filter_stream_(fs), index(0), is_gz_device_(false)
	{}

	template<class FWObjPara>
	void operator()(FWObjPara type)
	{
		typedef typename boost::mpl::back<LIST>::type SinkType;
		typedef FDevice<SharedMemoryType, 
					SinkType, 
					boost::mpl::at <SinkType, typename IoHandlerGlobalParameter::DeviceType>::type::value
					> SinkDevice;

		Closer<LIST, boost::mpl::size<LIST>::value, 9> gg(5566);

		typedef typename Closer<LIST, boost::mpl::size<LIST>::value, 9>::type gzip_type;

   		typedef FDevice<SharedMemoryType, 
					gzip_type,
					boost::mpl::at <gzip_type, typename IoHandlerGlobalParameter::DeviceType>::type::value
                    > CurrentDevice;


		if (boost::is_same < gzip_type, boost::mpl::map <boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::GzipCompressFilter> > > >::value ||
			boost::is_same < gzip_type, boost::mpl::map <boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::GzipDeCompressFilter> > > >::value )
			std::cerr<<"same type"<<'\n';
		else
			std::cerr<<"diff type"<<'\n';

		if (boost::mpl::at <FWObjPara, typename IoHandlerGlobalParameter::DeviceType>::type::value == FileDeviceType::GzipCompressFilter || 
			boost::mpl::at <FWObjPara, typename IoHandlerGlobalParameter::DeviceType>::type::value == FileDeviceType::GzipDeCompressFilter)
			(filter_stream_.template component <CurrentDevice> (index))->close 
				(*(filter_stream_.template component <SinkDevice> (boost::mpl::size<LIST>::value-1)),  std::ios_base::out);
		std::cerr<<"end"<<'\n';
		++index;
	}
};


template <class LIST, class SharedMemoryType, int N>
struct CR
{
	typedef typename boost::mpl::at< LIST, boost::mpl::int_<N-1> >::type cur_map_type;
	typedef typename boost::mpl::at <cur_map_type, IoHandlerGlobalParameter::DeviceType>::type cur_d_type;

	typedef typename boost::mpl::at< LIST, boost::mpl::int_<N> >::type post_map_type;
	typedef typename boost::mpl::at <post_map_type, IoHandlerGlobalParameter::DeviceType>::type post_d_type;

	template <typename FilterStream>
	CR (FilterStream& filter_stream_)                                                                                                                                            
	{  
		typedef FDevice<SharedMemoryType, 
					post_map_type, 
					post_d_type::value
					> SinkDevice;
		
		typedef FDevice<SharedMemoryType, 
					cur_map_type,
					cur_d_type::value
					> CurrentDevice;

/*		if (boost::is_same < gzip_type, boost::mpl::map <boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::GzipCompressFilter> > > >::value ||
			boost::is_same < gzip_type, boost::mpl::map <boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::GzipDeCompressFilter> > > >::value )
			std::cerr<<"same type"<<'\n';
		else
			std::cerr<<"diff type"<<'\n';

		if (boost::mpl::at <FWObjPara, typename IoHandlerGlobalParameter::DeviceType>::type::value == FileDeviceType::GzipCompressFilter || 
			boost::mpl::at <FWObjPara, typename IoHandlerGlobalParameter::DeviceType>::type::value == FileDeviceType::GzipDeCompressFilter)
*/
			CR <LIST, SharedMemoryType, N-1> kk (filter_stream_);
			(filter_stream_.template component <CurrentDevice> (N-1))->close 
				(*(filter_stream_.template component <SinkDevice> (N)),  std::ios_base::out);
	}   
};

template <class LIST, class SharedMemoryType>
struct CR <LIST, SharedMemoryType, 0> 
{
	template <typename FilterStream>
	CR (FilterStream& filter_stream_)
	{}
};




template<class SETTING, class LIST >//, class STREAM_TYPE = std::ostream>
struct iohandler
	: public boost::mpl::at < SETTING, typename IoHandlerGlobalParameter::BaseStreamType > ::type
{
private:
	
public:
	typedef SharedMemory<SETTING> SharedMemoryType;
	typedef typename boost::mpl::at <SETTING, typename IoHandlerGlobalParameter::FilteringStreamType>::type FilteringStreamType;	
	typedef typename SharedMemoryType::MutipleNumberType MutipleNumberType;
	typedef typename boost::mpl::at <SETTING, typename IoHandlerGlobalParameter::BaseStreamType >::type BaseStreamType;
	
	FilteringStreamType filter_stream_;
	SharedMemoryType shared_memory_;
	
	
	iohandler(typename SharedMemoryType::DeviceParameterType &para)
		: shared_memory_(boost::mpl::size<LIST>::value+1)
		, BaseStreamType (&filter_stream_)
	{
		shared_memory_.parameter = para;
		push_device();
	}

	iohandler()
		: shared_memory_(boost::mpl::size<LIST>::value+1)
		, BaseStreamType (&filter_stream_)
	{
		push_device();
	}
	
	void push_device()
	{
		boost::mpl::for_each<LIST>( PushDevice<FilteringStreamType, SharedMemoryType>(filter_stream_, shared_memory_) );
	}

	void close()
	{
		shared_memory_.flag_eof_ = true;
		filter_stream_.reset();
	}

	void bs_async_close()	//currently support basespace transmission only, and gz compress/decompress device is not supported
	{
		shared_memory_.flag_eof_ = true;
		typedef typename boost::mpl::back<LIST>::type ParameterType;

		typedef FDevice<SharedMemoryType, 
					ParameterType, 
					boost::mpl::at <ParameterType, typename IoHandlerGlobalParameter::DeviceType>::type::value//FileDeviceType::BasespaceDevice 
					> BSAsyncDevice;
//		while (!(filter_stream_.strict_sync()))
//		{
//			std::chrono::milliseconds dura ( 100 );
//			std::this_thread::sleep_for ( dura ); 	
//		}
		filter_stream_.strict_sync();
		filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1) -> close();
	}	


	void bs_async_close(int i)	//currently support basespace transmission only, and gz compress/decompress device is not supported
	{
		shared_memory_.flag_eof_ = true;

		this->flush();

		boost::mpl::for_each<LIST>( CloseDevice<FilteringStreamType, SharedMemoryType, LIST>(filter_stream_, shared_memory_));

		typedef typename boost::mpl::at< LIST,boost::mpl::int_<0> >::type FilterType;
		typedef FDevice<SharedMemoryType, 
					FilterType, 
					boost::mpl::at <FilterType, typename IoHandlerGlobalParameter::DeviceType>::type::value//FileDeviceType::BasespaceDevice 
					> GzFilter;

		typedef typename boost::mpl::back<LIST>::type ParameterType;
		typedef FDevice<SharedMemoryType, 
					ParameterType, 
					boost::mpl::at <ParameterType, typename IoHandlerGlobalParameter::DeviceType>::type::value//FileDeviceType::BasespaceDevice 
					> BSAsyncDevice;
//		std::cerr<<"strict_async pre: "<<filter_stream_.strict_sync()<<'\n';
//		this->flush();
//		std::cerr<<"strict_async post: "<<filter_stream_.strict_sync()<<'\n';

//flush: fail
//		auto yy = boost::iostreams::flush (* (filter_stream_.template component <GzFilter> (boost::mpl::size<LIST>::value-2)),
//		*filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1) );
//std::cerr<<"Flush gz "<<yy<<'\n';

//close success!!!
// (filter_stream_.template component <GzFilter> (boost::mpl::size<LIST>::value-2))->close (*(filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1)),  std::ios_base::out);
		//this->flush();
		filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1) -> close();
	}

	void bs_async_close(int i, int ii)	//currently support basespace transmission only, and gz compress/decompress device is not supported
	{
		shared_memory_.flag_eof_ = true;

		this->flush();

		CR <LIST, SharedMemoryType, boost::mpl::size<LIST>::value-1> Q (filter_stream_);
//		boost::mpl::for_each<LIST>( CloseDevice<FilteringStreamType, SharedMemoryType, LIST>(filter_stream_, shared_memory_));

		typedef typename boost::mpl::at< LIST,boost::mpl::int_<0> >::type FilterType;
		typedef FDevice<SharedMemoryType, 
					FilterType, 
					boost::mpl::at <FilterType, typename IoHandlerGlobalParameter::DeviceType>::type::value//FileDeviceType::BasespaceDevice 
					> GzFilter;

		typedef typename boost::mpl::back<LIST>::type ParameterType;
		typedef FDevice<SharedMemoryType, 
					ParameterType, 
					boost::mpl::at <ParameterType, typename IoHandlerGlobalParameter::DeviceType>::type::value//FileDeviceType::BasespaceDevice 
					> BSAsyncDevice;
//		std::cerr<<"strict_async pre: "<<filter_stream_.strict_sync()<<'\n';
//		this->flush();
//		std::cerr<<"strict_async post: "<<filter_stream_.strict_sync()<<'\n';

//flush: fail
//		auto yy = boost::iostreams::flush (* (filter_stream_.template component <GzFilter> (boost::mpl::size<LIST>::value-2)),
//		*filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1) );
//std::cerr<<"Flush gz "<<yy<<'\n';

//close success!!!
// (filter_stream_.template component <GzFilter> (boost::mpl::size<LIST>::value-2))->close (*(filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1)),  std::ios_base::out);
		//this->flush();
		filter_stream_.template component <BSAsyncDevice> (boost::mpl::size<LIST>::value-1) -> close();
	}
};

#endif
