///@file file_reader_impl.hpp
///@brief define generic format of IHandler_impl template class. 
///@author C-Salt Corp.
#ifndef IHANDLER_IMPL_HPP_
#define IHANDLER_IMPL_HPP_
#include <iosfwd>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "../../constant_def.hpp"
#include "../../format_io_iterator.hpp"
#include "ihandler_data_source.hpp"

/// @brief include member functions: get_next_entry (), get_all_entry ()\n
/// has a variatic parameter list, wherein only a template template parameter FORMAT and a type TUPLETYPE are identified.
/// @tparam template template parameter of format_types, indicating what the kind of format that the IHandler_impl is supposed to get.
/// @tparam TUPLETYPE indicating data format of the got piece of data
template < template < typename T > class FORMAT, typename TUPLETYPE, typename SOURCE_TYPE = IoHandlerBaseSpaceDownload >
class IHandler_impl 
	: public IoHandlerDataSource < SOURCE_TYPE >
{  
public: 
	/// @memberof IHandler_impl, for getting next entry from the ifstream object
    static class FORMAT < TUPLETYPE > get_next_entry (void)//(std::ifstream& file_handle)
	{}
	/// @memberof IHandler_impl, for getting next all entries from the file_name
};

//#include "format/fasta_ihandler_impl.hpp"
#include "format/fastq_ihandler_impl.hpp"
//#include "format/sam_ihandler_impl.hpp"
#endif
