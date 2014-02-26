/**
 * @file fastq_reader_impl.hpp
 * @brief Specialized version of class FileReader_impl for Fastq, for providing operation detail for getting a piece of FastQ data from an ifstream object
 * This class provides a function to read the fastq format file.
 *
 * @author JHH corp.
 */
#ifndef FASTQ_READER_IMPL_HPP_
#define FASTQ_READER_IMPL_HPP_
#include <tuple>
#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string/erase.hpp"
#include "fastq.hpp"
#include "../file_reader_impl.hpp"
#include "../tuple_utility.hpp"

/** * @brief Read Fastq format data, including member functions: io_end (), get_next_entry (), and get_all_entry ()
 * @tparam Fastq template template parameter Fastq indicating current FileReader_impl is a specilization form of the base FileReader_impl template class for Fastq format
 * @tparam TUPLETYPE indicating Fasta data format, defaulted as tuple < string, string, string, string >
 * 
 */
template< class TUPLETYPE, SOURCE_TYPE STYPE>//Specialization for FASTQ
class FileReader_impl <Fastq, TUPLETYPE, STYPE>
	: public DataSource < STYPE >
{
	public:
		typedef Fastq<TUPLETYPE> format_type;
		typedef TUPLETYPE tuple_type;
		typedef SOURCE_TYPE source_type; 
};

template< class TUPLETYPE>//Specialization for FASTQ
class FileReader_impl <Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE>
	: public DataSource < SOURCE_TYPE::IFSTREAM_TYPE >
{
public:
	size_t file_num_;
	typedef Fastq<TUPLETYPE> format_type;
	typedef TUPLETYPE tuple_type;
///@typedef io_iterator in format of formatIoIterator < Fastq <TUPLETYPE>, FileReader_impl <Fastq, TUPLETYPE> > 
	typedef FormatIoIterator<
							Fastq<TUPLETYPE>, 
							FileReader_impl<Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE >
							> io_iterator;	
/// @memberof FileReader_impl<Fastq, TUPLETYPE>
/// @brief return a pointer pointing to a Fastq <TUPLETYPE> with its eof_flag setted as true
/// @return shared_ptr < Fastq <TUPLETYPE> > (new Fastq <TUPLETYPE> (true) )
	io_iterator io_end ()
	{   
		return io_iterator (std::shared_ptr < Fastq <TUPLETYPE> > (new Fastq <TUPLETYPE> (true) ) );
	}
   
	FileReader_impl ()
		: DataSource < SOURCE_TYPE::IFSTREAM_TYPE > ()
		, file_num_ (0)
	{}

	FileReader_impl (std::vector<std::string>& file_path, std::vector <uint64_t> sizein = std::vector<uint64_t>(0) )
		: DataSource < SOURCE_TYPE::IFSTREAM_TYPE > (file_path, sizein)
		, file_num_ ( file_path.size() )
	{}

/// @memberof FileReader_impl<Fastq, TUPLETYPE>
/// @brief return an object of Fastq <TUPLETYPE> by means of reading an std::ifstream& object; \n
/// @param file_handle an std::ifstream& object 
/// @return Fastq <TUPLETYPE> 
	Fastq<TUPLETYPE> get_next_entry (size_t i)//std::ifstream& file_handle)
	{   
		std::string name1, name2, sequence, quality;
		name1.reserve (256);
		name2.reserve (256);
		sequence.reserve (1024);
		quality.reserve (1024);

		bool return_flag = false;
		Fastq<TUPLETYPE> result_element;

		this -> file_handle [i] -> peek();
		if ( (this -> file_handle[i]) -> fail() || 
				(this -> file_handle[i]) -> eof()  || 
				(this -> file_handle[i]) -> bad()  ) //fail
		{
			result_element.eof_flag = true;
			return result_element;
		}
		std::getline ( *(this -> file_handle[i]), name1);
		std::getline ( *(this -> file_handle[i]), sequence);  
		std::getline ( *(this -> file_handle[i]), name2);  
		std::getline ( *(this -> file_handle[i]), quality); 

		if ( !(this -> file_handle[i]) -> good() )
		{
			result_element.eof_flag = true;
			return result_element;
		}
		boost::trim (name1);
		boost::trim (sequence);
		boost::trim (name2);
		boost::trim (quality);
		result_element.data = std::make_tuple (name1, sequence, name2, quality);
		return result_element;
	};  

};

#endif
