#ifndef FW_DEVICE_CONTAINER_HPP_
#define FW_DEVICE_CONTAINER_HPP_

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
#include "../iohandler.hpp"                                                                                                                                                              

template<typename Container>
class ContainerDevice
{
private:
	typedef typename Container::size_type size_type;
	Container& container_;
	size_type pos_;
public:
	
	typedef typename Container::value_type char_type;
	typedef boost::iostreams::seekable_device_tag category;
	bool &is_close_;
	
	ContainerDevice(Container& container, bool &ic)
		: container_(container)
		, pos_(0)
		, is_close_(ic)
	{}

	std::streamsize read(char_type* s, std::streamsize n)
	{
		std::cout << "read" << n << std::endl;
		using namespace std;
		streamsize amt = static_cast<streamsize>(container_.size() - pos_);
		streamsize result = (min)(n, amt);
		if (result != 0) {
			std::copy( container_.begin() + pos_, 
					   container_.begin() + pos_ + result, 
					   s );
			pos_ += result;
			return result;
		} else {
			if( is_close_  )
				return -1; // EOF
			else
				return 0;
		}
	}
	std::streamsize write(const char_type* s, std::streamsize n)
	{
		std::cout << "write" << n << std::endl;
		using namespace std;
		streamsize result = 0;
		if (pos_ != container_.size()) {
			streamsize amt = 
				static_cast<streamsize>(container_.size() - pos_);
			result = (min)(n, amt);
			std::copy(s, s + result, container_.begin() + pos_);
			pos_ += result;
		}
		if (result < n) {
			container_.insert(container_.end(), s + result, s + n);
			pos_ = container_.size();
		}
		return n;
	}
	boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
	{
		std::cout << "seek" << off << std::endl;
		using namespace std;

		// Determine new value of pos_
		boost::iostreams::stream_offset next;
		if (way == ios_base::beg) {
			next = off;
		} else if (way == ios_base::cur) {
			next = pos_ + off;
		} else if (way == ios_base::end) {
			next = container_.size() + off - 1;
		} else {
			throw ios_base::failure("bad seek direction");
		}

		// Check for errors
		if (next < 0 || next >= container_.size())
			throw ios_base::failure("bad seek offset");

		pos_ = next;
		std::cout << "seek pos" << pos_ << std::endl;
		return pos_;
	}
	template<typename T>	 
	void close(T& t, BOOST_IOS::openmode m)
	{
		std::cout << "CLOSE" << std::endl;
	}
	Container& container() { return container_; }

};
#endif
