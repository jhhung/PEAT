/**																																								  
 *  @file data_source_impl.hpp
 *  @brief provide interface class to select from file_path and curl data sources 
 *  @author C-Salt Corp.
 */
#ifndef IHANDLER_DATA_SOURCE_HPP_
#define IHANDLER_DATA_SOURCE_HPP_  
#include "../iohandler.hpp"
#include "../basespace_def.hpp"

template< typename SOURCE_TYPE >
class IoHandlerDataSource_impl
{
public:
    size_t source_num_;
    std::vector <uint64_t> file_size_;
    std::vector < SOURCE_TYPE* > file_handle;
	
	IoHandlerDataSource_impl ( std::vector <std::string> file_path, std::vector <uint64_t> sizein )
	: source_num_ ( file_path.size () )
    , file_size_ (sizein)
    {
        if (sizein.size()!=file_path.size())
           	for ( auto& i : file_path )
            {
                DeviceParameter dp;
                dp.bs_download_url_  = i;
                dp.bs_download_size_ = 0;
               	file_handle.emplace_back ( new SOURCE_TYPE ( dp ) );
            }
      	else
           	for (auto idx=0; idx!=file_path.size(); ++idx)
            {
                DeviceParameter dp;
               	dp.bs_download_url_ = file_path[idx];
                dp.bs_download_size_ = sizein[idx];
                file_handle.emplace_back ( new SOURCE_TYPE ( dp ) );
            }
    }
			
	~IoHandlerDataSource_impl()
	{
		for ( auto& j : file_handle )
		{
			delete j;
		}
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
// * @class IoHandlerDataSource
// * @brief provide generic form of data source
// * @tparam SOURCE_TYPE an enum served as an interface to indicate the data source is a curl_gz_format, curl_plain_format, or a ifstream_plain_format
// */
template <typename SOURCE_TYPE = IoHandlerBaseSpaceDownload >
class IoHandlerDataSource 
	: public IoHandlerDataSource_impl<SOURCE_TYPE> 
{
public:
	IoHandlerDataSource ( std::vector <std::string> file_path, std::vector <uint64_t> sizein )
		: IoHandlerDataSource_impl <SOURCE_TYPE> ( file_path, sizein )
	{}	
};

//specialize for IoHandlerBaseSpaceDownload
template<>
class IoHandlerDataSource< IoHandlerBaseSpaceDownload >
	:public IoHandlerDataSource_impl< IoHandlerBaseSpaceDownload >
{
public:	
	IoHandlerDataSource ( std::vector <std::string> file_path, std::vector <uint64_t> sizein )
		: IoHandlerDataSource_impl <IoHandlerBaseSpaceDownload> ( file_path, sizein )
	{}
	~IoHandlerDataSource()
	{
		std::string line;
		for ( auto source : file_handle ) 
		{
			if(source == nullptr)
				continue;
			while( std::getline ( *source, line ) );
			//delete source;
		}
	}	
};
#endif
