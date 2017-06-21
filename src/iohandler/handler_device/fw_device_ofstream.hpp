#ifndef FW_DEVICE_OFSTREAM_HPP_
#define FW_DEVICE_OFSTREAM_HPP_

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include "../iohandler.hpp"                                                                                                                                                              

template<class SharedMemoryType, class ParaType>
struct FDevice<SharedMemoryType, ParaType, FileDeviceType::Ofstream>
	:public boost::iostreams::file_descriptor_sink
{
	SharedMemoryType &shared_memory_;
	FDevice(SharedMemoryType &sm)
		: boost::iostreams::file_descriptor_sink(sm.parameter.filename)
		, shared_memory_(sm)
	{}
	~FDevice()
	{}
};

#endif
