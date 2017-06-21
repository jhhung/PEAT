#ifndef FW_FILTER_GZIP_COMPRESSOR_HPP_
#define FW_FILTER_GZIP_COMPRESSOR_HPP_
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include "../iohandler.hpp"

template<class SharedMemoryType, class ParaType>
struct FDevice<SharedMemoryType, ParaType, FileDeviceType::GzipCompressFilter>
	:public boost::iostreams::gzip_compressor
{
	SharedMemoryType &shared_memory_;
public:
	FDevice(SharedMemoryType &sm)
		: boost::iostreams::gzip_compressor()
		, shared_memory_(sm)
	{}
	~FDevice()
	{}
};

#endif
