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
//		std::vector < Fastq<TUPLETYPE> > result (file_num_);

		bool return_flag = false;
//		for ( auto i = 0; i != file_num_; ++i )
//		{
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
			//		boost::erase_all (name1, " ");
			//		boost::erase_all (sequence, " ");
			//		boost::erase_all (name2, " ");
			//		boost::erase_all (quality, " ");
			result_element.data = std::make_tuple (name1, sequence, name2, quality);
//			result[i] = (result_element);
//		}
		return result_element;
	};  

/// @memberof FileReader_impl<Fastq, TUPLETYPE>
/// @brief receive a std::string of file_name, and accordingly read all Fastq <TUPLETYPE> entries thereof and update the call by reference parameter of data_cluster to record the got Fastq <TUPLETYPE> data entries
/// @param file_name in format of std::string 
/// @param data_cluster passed into get_all_entry function by reference, so as to keep the got Fastq <TUPLETYPE> data entries
	//static 
	void get_all_entry (//std::string& file_name, 
						size_t index, std::vector< Fastq<TUPLETYPE> > & result)
						//std::vector <std::vector< Fastq<TUPLETYPE> > >& result)
	{
		//result.clear ();
		result.resize (0);//file_num_);
		//std::ifstream file_handle (file_name);
		while (true)
		{
			//std::vector < Fastq<TUPLETYPE> > x = get_next_entry (); //(file_handle);
			result.emplace_back ( get_next_entry (index) ); //(file_handle);
//			bool all_eof = true;
//			for ( auto i = 0; i != file_num_; ++i )
//			{
//				result[i].push_back (x[i]);
//				all_eof *= x[i].eof_flag;
//			}
			if (result.back().eof_flag)
//			if (all_eof)
				break;
		}
	}
};

template< class TUPLETYPE, SOURCE_TYPE STYPE >//curl_gz & curl_plain specialization for FASTQ
class FileReader_impl <Fastq, TUPLETYPE, STYPE, curl_default_handle_mutex >
	: public DataSource < STYPE, curl_default_handle_mutex >
{
public:
typedef Fastq<TUPLETYPE> format_type;
typedef TUPLETYPE tuple_type;
typedef curl_default_handle_mutex curl_handle_type;
	size_t file_num_;
	bool end_;
	typedef FormatIoIterator<
							Fastq<TUPLETYPE>, 
							FileReader_impl<Fastq, TUPLETYPE, STYPE >//IFSTREAM_TYPE >
							> io_iterator;	

	io_iterator io_end ()
	{   
		return io_iterator (std::shared_ptr < Fastq <TUPLETYPE> > (new Fastq <TUPLETYPE> (true) ) );
	}

	FileReader_impl ()
		: DataSource < STYPE, curl_default_handle_mutex > ()
		, file_num_ (0)
		, end_ (false)
	{}

	FileReader_impl ( std::vector <std::string>& url, std::vector <uint64_t> gz_file_size )
		: DataSource < STYPE, curl_default_handle_mutex > ( url, gz_file_size )
		, file_num_ ( url.size() )
		, end_ (false)
	{}

	Fastq<TUPLETYPE> get_next_entry (size_t index)
	{
		bool yy;
		std::string name1 = this -> source_get_line(index, yy);
		std::string sequence = this -> source_get_line(index);
		std::string name2 = this -> source_get_line(index);
		std::string quality = this -> source_get_line(index);

		Fastq<TUPLETYPE> temp_element;
		if (yy)
			temp_element.eof_flag = true;
		else
		{
			boost::trim (name1);
			boost::trim (sequence);
			boost::trim (name2);
			boost::trim (quality);
			temp_element.data = std::make_tuple (name1, sequence, name2, quality);
		}
		return temp_element;
	};  

	std::vector < Fastq<TUPLETYPE> >  get_next_entry (void)
	{
		std::vector < Fastq<TUPLETYPE> > result (file_num_);//(0);
		for ( auto index=0; index!=file_num_; ++index)
			result.push_back ( get_next_entry (index) );
		return result;
	};  

	void get_all_entry ( std::vector < std::vector< Fastq<TUPLETYPE> > >& result)
	{
		result.clear ();
		result.resize (file_num_);

		while (true)
		{
			std::vector < Fastq<TUPLETYPE> > x = get_next_entry (); //(file_handle);
			bool all_eof = true;//false;
			for ( auto i = 0; i != file_num_; ++i )
				all_eof *= x[i].eof_flag;
			if (all_eof)
				break;
			for ( auto i = 0; i != file_num_; ++i )
				result[i].push_back (x[i]);
		}
	}
};

/*	move to barcode_handler class
// impl for barcode version
#include "fastq_barcode.hpp"

template< class TUPLETYPE>//Specialization for FASTQ
class FileReader_impl <FastqBarcode, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE>
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
	size_t barcode_length_;

/// @memberof FileReader_impl<Fastq, TUPLETYPE>
/// @brief return a pointer pointing to a Fastq <TUPLETYPE> with its eof_flag setted as true
/// @return shared_ptr < Fastq <TUPLETYPE> > (new Fastq <TUPLETYPE> (true) )
	io_iterator io_end ()
	{   
		return io_iterator (std::shared_ptr < Fastq <TUPLETYPE> > (new Fastq <TUPLETYPE> (true) ) );
	}
   
	FileReader_impl (size_t barcode_len=0)
		: DataSource < SOURCE_TYPE::IFSTREAM_TYPE > ()
		, file_num_ (0)
		, barcode_length_ (barcode_len)
	{}

	FileReader_impl (std::vector<std::string>& file_path, size_t barcode_len=0, std::vector <uint64_t> sizein = std::vector<uint64_t>(0) )
		: DataSource < SOURCE_TYPE::IFSTREAM_TYPE > (file_path, sizein)
		, file_num_ ( file_path.size() )
		, barcode_length_ (barcode_len)
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
		if ((this -> file_handle[i]) -> fail() || 
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
		boost::trim (name1);
		boost::trim (sequence);
		boost::trim (name2);
		boost::trim (quality);

		sequence.replace(0, barcode_length_, "");// trimming barcode off from sequence string
		quality.replace(0, barcode_length_, "");// trimming barcode off from quality string

		result_element.data = std::make_tuple (name1, sequence, name2, quality);
		return result_element;
	};  
};

template< class TUPLETYPE, SOURCE_TYPE STYPE >//curl_gz & curl_plain specialization for FASTQ
class FileReader_impl <FastqBarcode, TUPLETYPE, STYPE, curl_default_handle_mutex >
	: public DataSource < STYPE, curl_default_handle_mutex >
{
public:
typedef Fastq<TUPLETYPE> format_type;
typedef TUPLETYPE tuple_type;
typedef curl_default_handle_mutex curl_handle_type;
	size_t file_num_;
	bool end_;
	typedef FormatIoIterator<
							Fastq<TUPLETYPE>, 
							FileReader_impl<Fastq, TUPLETYPE, STYPE >//IFSTREAM_TYPE >
							> io_iterator;	
	size_t barcode_length_;

	io_iterator io_end ()
	{   
		return io_iterator (std::shared_ptr < Fastq <TUPLETYPE> > (new Fastq <TUPLETYPE> (true) ) );
	}

	FileReader_impl (size_t barcode_len=0)
		: DataSource < STYPE, curl_default_handle_mutex > ()
		, file_num_ (0)
		, end_ (false)
		, barcode_length_ (barcode_len)
	{}

	FileReader_impl ( std::vector <std::string>& url, std::vector <uint64_t> gz_file_size, size_t barcode_len=0 )
		: DataSource < STYPE, curl_default_handle_mutex > ( url, gz_file_size )
		, file_num_ ( url.size() )
		, end_ (false)
		, barcode_length_ (barcode_len)
	{}

	Fastq<TUPLETYPE> get_next_entry (size_t index)
	{
		bool yy;
		std::string name1 = this -> source_get_line(index, yy);
		std::string sequence = this -> source_get_line(index);
		std::string name2 = this -> source_get_line(index);
		std::string quality = this -> source_get_line(index);

		Fastq<TUPLETYPE> temp_element;
		if (yy)
			temp_element.eof_flag = true;
		else
		{
			boost::trim (name1);
			boost::trim (sequence);
			boost::trim (name2);
			boost::trim (quality);
			sequence.replace(0, barcode_length_, "");// trimming barcode off from sequence string
			quality.replace(0, barcode_length_, "");// trimming barcode off from quality string
			temp_element.data = std::make_tuple (name1, sequence, name2, quality);
		}
		return temp_element;
	};  

	std::vector < Fastq<TUPLETYPE> >  get_next_entry (void)
	{
		std::vector < Fastq<TUPLETYPE> > result (file_num_);//(0);
		for ( auto index=0; index!=file_num_; ++index)
			result.push_back ( get_next_entry (index) );
		return result;
	};  
};
*/
#endif
