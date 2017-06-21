#ifndef FW_FILTER_GZIP_DECOMPRESSOR_HPP_
#define FW_FILTER_GZIP_DECOMPRESSOR_HPP_
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include "../iohandler.hpp"

template<class SharedMemoryType, class ParaType>
struct FDevice<SharedMemoryType, ParaType, FileDeviceType::GzipDeCompressFilter>
	:public boost::iostreams::gzip_decompressor
{
	SharedMemoryType &shared_memory_;
public:
	FDevice(SharedMemoryType &sm)
		: boost::iostreams::gzip_decompressor()
		, shared_memory_(sm)
	{
		std::cout<< "gz decompress call\n"; 
	}
	~FDevice()
	{}
};

#endif
