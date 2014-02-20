///@file file_reader_impl.hpp
///@brief define generic format of FileReader_impl template class. 
///@author C-Salt Corp.
#ifndef FILE_READER_IMPL_HPP_
#define FILE_READER_IMPL_HPP_
#include <iosfwd>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "constant_def.hpp"
#include "format_io_iterator.hpp"
#include "format/data_source_impl.hpp"

/// @brief include member functions: get_next_entry (), get_all_entry ()\n
/// has a variatic parameter list, wherein only a template template parameter FORMAT and a type TUPLETYPE are identified.
/// @tparam template template parameter of format_types, indicating what the kind of format that the FileReader_impl is supposed to get.
/// @tparam TUPLETYPE indicating data format of the got piece of data
template < template < typename T > class FORMAT, typename TUPLETYPE, SOURCE_TYPE STYPE, typename... ARGS >
struct FileReader_impl 
	: public DataSource < STYPE, ARGS... >
{    
	/// @memberof FileReader_impl, for getting next entry from the ifstream object
    static class FORMAT < TUPLETYPE > get_next_entry (void)//(std::ifstream& file_handle)
	{}
	/// @memberof FileReader_impl, for getting next all entries from the file_name
//    static void  get_all_entry (std::string file_name, std::vector < FORMAT <TUPLETYPE> >& data_cluster )
//    {
//	return nullptr;
//    }
};


//#include "format/fasta_reader_impl.hpp"
#include "format/fastq_reader_impl.hpp"
//#include "format/bed_reader_impl.hpp"
//#include "format/wig_reader_impl.hpp"
//#include "format/sam_reader_impl.hpp"
#endif
