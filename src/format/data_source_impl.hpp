/**																																								  
 *  @file data_source_impl.hpp
 *  @brief provide interface class to select from file_path and curl data sources 
 *  @author C-Salt Corp.
 */
#ifndef DATA_SOURCE_IMPL_HPP_
#define DATA_SOURCE_IMPL_HPP_  
//#include "curl_recv_wrapper.hpp"

/**
 * @class DataSource
 * @brief provide generic form of data source
 * @tparam SOURCE_TYPE an enum served as an interface to indicate the data source is a curl_gz_format, curl_plain_format, or a ifstream_plain_format
 */
template < SOURCE_TYPE, typename... >
class DataSource 
{};

/**
 * @class DataSource < SOURCE_TYPE::IFSTREAM_TYPE >
 * @brief Specialized version of class DataSource exclusively for the situation that the data sources are in the format of ifstream
 */
template <>
class DataSource < SOURCE_TYPE::IFSTREAM_TYPE >
{
public:
	size_t source_num_;
	std::vector <uint64_t> file_size_;
	std::vector < std::ifstream* > file_handle;

/**
 * @brief constructor
 */
	DataSource ()
	{}

	~DataSource (void)
	{
		for ( auto& j : file_handle )
			delete j;
	}
/**
 * @brief constructor
 */
	DataSource ( std::vector <std::string> file_path, std::vector <uint64_t> sizein = std::vector<uint64_t>(0) )
		: source_num_ ( file_path.size () )
		, file_size_ (sizein)
	{
		for ( auto& i : file_path )
			file_handle.push_back ( new std::ifstream( i ) );																								
	}

/**
 * @brief main interface to get line from the index-th file_handle and return a std::string
 */
	std::string source_get_line (size_t index)
	{
		std::string temp;
		std::getline ( (*file_handle[index]), temp );
		return temp;
	}

/**
 * @brief main interface to get line from each of the file_handle and return a std::vector<std::string>
 */
	std::vector<std::string> source_get_line (void)
	{
		std::vector < std::string> return_vec;
		std::string temp;
		for ( auto i : file_handle )
		{
			std::getline ( (*i), temp );
			return_vec.push_back (temp);
		}
		return return_vec;
	}
};

///**
// * @class DataSource < SOURCE_TYPE::CURL_TYPE_GZ, curl_default_handle_mutex >
// * @brief specialization for curl gz format data source, with the non-type enum specialization to be SOURCE_TYPE::CURL_TYPE_GZ
// */
//template <>
//class DataSource < SOURCE_TYPE::CURL_TYPE_GZ, curl_default_handle_mutex >
//{
//public:
//	size_t source_num_;
//	std::vector <bool> end_of_curl_, initial_flag_;
//	std::vector < CurlWrapper < CompressFormat::GZ, curl_default_handle_mutex >* > Curl_device_; 
//	std::vector < std::stringstream* > file_handle;
//
///**
// * @brief default constructor with no arguments
// */
//	DataSource ()
//		: Curl_device_ (0)
//		, end_of_curl_ (0)
//		, initial_flag_ (0)
//	{
//		INITIALIZE_CURL ();
//	}
//
///**
// * @brief constructor with std::vector of url and gz_file_size and execute INITIALIZE_CURL () function
// */
//	DataSource ( std::vector <std::string>& url, std::vector <uint64_t> gz_file_size = std::vector<uint64_t> (0) )
//		: source_num_ ( url.size() )
//		, end_of_curl_ (source_num_, false)
//		, initial_flag_ (source_num_, false)
//	{
//		INITIALIZE_CURL ();
//		if (gz_file_size.size()<url.size())
//			gz_file_size.resize (url.size());
//		for ( auto i = 0; i != source_num_; ++i )
//		{
//			 Curl_device_.push_back ( new 
//				CurlWrapper <CompressFormat::GZ, curl_default_handle_mutex >  
//					( url[i], gz_file_size[i]) );
//			file_handle.push_back ( new std::stringstream );
//		}
//	}
//
///**
// * @brief destructor having newed ptr type Curl_device_ and file_handle deleted, and executing the curl_global_cleanup function
// */
//	~DataSource (void)
//	{
//		for ( auto& i : Curl_device_ )
//			delete i;
//		for ( auto& j : file_handle )
//			delete j;
//		curl_global_cleanup();
//	}
//
///**
// * @brief main function to get std::string from file_handle.
// * @param index indicate which file_handle is going to be accesssed.
// */
//	//single source version
//	std::string source_get_line (size_t index)
//	{
//		if ( end_of_curl_[index] )	//return empty std::string when end_of_curl_[index] flag has been set
//			return "";
//		if (! initial_flag_[index] )	
//		{//activate function get_from_url_impl(), getting data from curl to inflate buffer, and parse, getting data from inflate buffer to file_handle[index] 
//			Curl_device_[index] -> get_from_url_impl ();
//			Curl_device_[index] -> parse ( *(file_handle[index]) );
//			initial_flag_[index] = true;
//		}
//		if ( Curl_device_[index]->terminate_flag_ )
//		{//check whether file_handle[index] has reached eof criterion when the terminate_flag_ of the Curl_device_[index] has been set, if so, clear file_handle[index] and set end_of_curl_[index] flag and return empty std::string
//			file_handle[index] -> peek();
//			if ( file_handle[index]->fail() || file_handle[index]->eof() || file_handle[index]->bad() ) 
//			{
//				end_of_curl_[index] = true;
//				file_handle[index]->str("");
//				return "";
//			}
//		}
//		int offset_1 = file_handle[index]->tellg();
//		int offset_2 = file_handle[index]->tellp();
//		if ( !Curl_device_[index]->terminate_flag_ && offset_2-offset_1 < second_buf_size )
//			Curl_device_[index] -> parse ( *(file_handle[index]) );
//		std::string temp;
//		std::getline ( *(file_handle[index]), temp );
//		return temp;
//	}
//
///**
// * @brief an overloaded version to have a pass by ref bool set according to the value of end_of_curl_[index] flag
// */
//	std::string source_get_line (size_t index, bool& end)
//	{//with an extra pass by ref bool to return the end_of_curl_[index] flag
//		std::string result = source_get_line (index);
//		end = end_of_curl_[index];
//		return result;
//	}
//
//	std::vector <std::string> source_get_line (void)
//	{//parallel version to get a std::string from each of the file_handles
//		std::vector <std::string> result;
//		for ( auto index = 0; index != source_num_; ++index )
//			result.emplace_back ( source_get_line (index) );
//		return result;
//	}
//
//	std::vector <std::string> source_get_line (std::vector <bool>& end)	
//	{
//		end.clear();
//		end.resize (source_num_);
//		std::vector <std::string> result = source_get_line ();
//		end = end_of_curl_;
//		return result;
//	}
//};
//
///**
// * @class DataSource < SOURCE_TYPE::CURL_TYPE_PLAIN, curl_default_handle_mutex >
// * @brief specialization for curl plain format data source, with the non-type enum specialization to be SOURCE_TYPE::CURL_TYPE_PLAIN
// */
//template <>
//class DataSource < SOURCE_TYPE::CURL_TYPE_PLAIN, curl_default_handle_mutex >
//{
//public:
//	size_t source_num_;
//	std::vector <bool> 
//	end_of_curl_, initial_flag_;
//	std::vector < CurlWrapper < CompressFormat::PLAIN, curl_default_handle_mutex >* > Curl_device_; 
//	std::vector < std::stringstream* > file_handle;
//
///**
// * @brief default constructor
// */
//	DataSource ()
//		: Curl_device_ (0)
//		, end_of_curl_ (0)
//		, initial_flag_ (0)
//	{
//		INITIALIZE_CURL ();
//	}
//
///**
// * @brief constructor with std::vector of url and std::vector of size, which is defaulted with size 0.
// */
//	DataSource ( std::vector <std::string>& url, std::vector <uint64_t> file_size = std::vector<uint64_t>(0) )
//		: source_num_ ( url.size() )
//		, end_of_curl_ (source_num_, false)
//		, initial_flag_ (source_num_, false)
//	{
//		INITIALIZE_CURL ();
//		if (file_size.size()<url.size())
//			file_size.resize (url.size());
//		for ( auto i = 0; i != source_num_; ++i )
//		{
//			 Curl_device_.push_back ( new 
//				CurlWrapper <CompressFormat::PLAIN, curl_default_handle_mutex >  
//					( url[i], file_size[i] ) );
//			file_handle.push_back ( new std::stringstream );
//		}
//	}
//
///**
// * @brief destructor having newed ptr type Curl_device_ and file_handle deleted, and executing the curl_global_cleanup function
// */
//	~DataSource (void)
//	{
//		for ( auto& i : Curl_device_ )
//			delete i;
//		for ( auto& j : file_handle )
//			delete j;
//		curl_global_cleanup();
//	}
//
///**
// * @brief main function to get std::string from file_handle.
// * @param index indicate which file_handle is going to be accesssed.
// */
//	//single source version
//	std::string source_get_line (size_t index)
//	{
//		if ( end_of_curl_[index] )	//return empty std::string when end_of_curl_[index] flag has been set
//			return "";
//		if (! initial_flag_[index] )
//		{//activate function get_from_url_impl(), getting data from curl to inflate buffer, and parse, getting data from inflate buffer to file_handle[index] 
//			Curl_device_[index] -> get_from_url_impl ();
//			Curl_device_[index] -> parse ( *(file_handle[index]) );
//			initial_flag_[index] = true;
//		}
//		if ( Curl_device_[index]->terminate_flag_ )
//		{//check whether file_handle[index] has reached eof criterion when the terminate_flag_ of the Curl_device_[index] has been set, if so, clear file_handle[index] and set end_of_curl_[index] flag and return empty std::string
//			file_handle[index] -> peek();
//			if ( file_handle[index]->fail() || file_handle[index]->eof() || file_handle[index]->bad() ) 
//			{
//				end_of_curl_[index] = true;
//				file_handle[index]->str("");
//				return "";
//			}
//		}
//		int offset_1 = file_handle[index]->tellg();
//		int offset_2 = file_handle[index]->tellp();
//		if ( !Curl_device_[index]->terminate_flag_ && offset_2-offset_1 < second_buf_size )
//			Curl_device_[index] -> parse ( *(file_handle[index]) );
//		std::string temp;
//		std::getline ( *(file_handle[index]), temp );
//		return temp;
//	}
//
//	std::string source_get_line (size_t index, bool& end)//std::vector <bool>& end)
//	{
//		//end.clear();
//		//end.resize (source_num_);
//		//std::vector <std::string> result = source_get_line ();
//		std::string result = source_get_line (index);
//		end = end_of_curl_[index];
//		return result;
//	}
//
//	std::vector <std::string> source_get_line (void)	// sequential version
//	{
//		std::vector <std::string> result;
//		for ( auto index = 0; index != source_num_; ++index )
//			result.emplace_back ( source_get_line (index) );
//		return result;
//	}
//
//	std::vector <std::string> source_get_line (std::vector <bool>& end)	//sequential version
//	{
//		end.clear();
//		end.resize (source_num_);
//		std::vector <std::string> result = source_get_line ();
//		end = end_of_curl_;
//		return result;
//	}
//};
#endif
