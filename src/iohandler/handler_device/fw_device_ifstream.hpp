#ifndef FW_DEVICE_IFSTREAM_HPP_
#define FW_DEVICE_IFSTREAM_HPP_

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include "../iohandler.hpp"                                                                                                                                                              

template<class SharedMemoryType, class ParaType>
struct FDevice<SharedMemoryType, ParaType, FileDeviceType::Ifstream>
	:public boost::iostreams::file_descriptor_source//boost::iostreams::file_descriptor_sink
{
	SharedMemoryType &shared_memory_;
	FDevice(SharedMemoryType &sm)
		: boost::iostreams::file_descriptor_source(sm.parameter.bs_download_url_)//in_filename)
		, shared_memory_(sm)
	{}
	~FDevice()
	{}
};

#endif
