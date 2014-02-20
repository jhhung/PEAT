/**
 *  @file curl_recv_wrapper.hpp
 *  @brief provide wrapper class for curl receiving related detail 
 *  @author C-Salt Corp.
 */
#ifndef CURL_RECV_WRAPPER_HPP_
#define CURL_RECV_WRAPPER_HPP_
#include "curl_recv_impl.hpp"

/**
 * @class CurlWrapper
 * @brief provide a wrapper class that provide an interface to access and parse data obtained via curl. 
 * @tparam CompressFormat an enum served as an interface to indicate the format with which the curl data are provided, such as gz compressed, plain, and so on.  
 * @tparam FUNC indicate which call_back functor is going to be used in the CurRecvImpl::recv_impl function 
 */
template < CompressFormat, typename FUNC = curl_default_handle_mutex >
class CurlWrapper
{};

/**
 * @class CurlWrapper <CompressFormat::GZ, FUNC>
 * @brief Specialized version of class CurlWrapper exclusively for the situation that the curl data are in the format of gz compressed.
 */
template < typename FUNC >
class CurlWrapper < CompressFormat::GZ, FUNC >
{
public:
	std::string url_; //the to be curled url address
	uint64_t gz_file_size_; //the file size of the to be curled data
	std::stringstream inflate_result_stream_; //the data structure used in the gz_device_pipeline_ to hold the inflated data by gz deflation operation
	std::shared_ptr<std::stringstream> os_ptr_;//the data structure used by CurlRecvImpl to hold the curl data 
	boost::iostreams::filtering_streambuf<boost::iostreams::input> gz_device_pipeline_; //the gz inflate device that handling gz inflation
	uint64_t residue_, file_length_; //member variables used for estimation the inflate size of the curl data
	std::deque <std::thread> curl_in_thread_vec_; //
	std::future <size_t> fut_; //future objects served as a progress flags of the threading function, indicating that certain variables that will be altered by threading function, e.g. os_ptr_, are 100% ready to be access by the main function
	std::future <size_t> fut2_; 
	bool terminate_flag_;
	uint64_t got_size_;

public:
/**
 * @brief constructor 
 */
	CurlWrapper (void)
	{}

/**
 * @brief constructor 
 * @param url the to be curled url address
 * @param the file size of the curl data, counted in gz format
 */
	CurlWrapper (const std::string& url, uint64_t gz_file_size = 0)
		: url_ (url)
		, gz_file_size_ (gz_file_size)
		, inflate_result_stream_ ()
		, os_ptr_()
		, gz_device_pipeline_ ()
		, curl_in_thread_vec_ (0)
		, terminate_flag_ (false)
		, got_size_(0)
	{
		gz_device_pipeline_.push ( boost::iostreams::gzip_decompressor() );
//		set_file_length ();
		this -> gz_device_pipeline_.push ( inflate_result_stream_, second_buf_size);//1000000);
	}

/**
 * @brief main interface to start the curl download operation.  A threading function is used to run the recv_impl function without blocking the operation of the main function.\n A while (true) loop is in the threading function used to keep calling recv_impl function to get curl data in part. Local variable length_r holds the length of data received in this recv_impl run. got_length is += in each recv_impl run, to signify the total length of received data, as well as the get in part start position in the next recv_impl run.  
 */
	void get_from_url_impl (void)//(size_t index)//(void) 
	{
		std::shared_ptr < std::promise <size_t> > sptr ( new ( std::promise <size_t> ) );
		fut_ = sptr->get_future();
		std::shared_ptr < std::promise <size_t> > sptr2 ( new ( std::promise <size_t> ) );
		fut2_ = sptr2->get_future();
		curl_in_thread_vec_.push_back ( 
			std::move (
				std::thread (
					[this, sptr, sptr2] ()
					{
						uint64_t got_length (0), length_r (0);
            	        CurlRecvImpl <&QQQ> QQ;
            	        os_ptr_ = QQ.get_result();
            	        sptr2 -> set_value (5566);  //indicating that the os_ptr_ is ready to be access, 
													//otherwise, data race situation might occurs on os_ptr_ in parse function 
						if (gz_file_size_!=0)//impl for situation that user prvide gz_file_size, so that retransmission can be facilitated
						{
							while (true)
							{
								got_length += length_r;
								if (got_length == gz_file_size_)
								//{
								//	std::cerr<<"length_receved "<<got_length<<std::endl;
									break;
								//}
								std::stringstream Qs;
								Qs << "Range: bytes="<<got_length<<"-";
								length_r = QQ.recv_impl ( this->url_, Qs.str() );
							}
	            	        sptr -> set_value (5566);  //indicating that the download operation of recv_impl has been achieved 
													   //a future object of this promise object is checked in parse function
	            	        std::cerr<<"DONE CURL GET with total recv length of "<<got_length<<'\n';//length_vec[index]<<'\n'; 
						}
						else	//impl for situation that user does not provide gz_file_size, i.e. without retransmission impl
						{
							got_length = QQ.recv_impl ( this -> url_, "");//get in part not supported here
							sptr -> set_value (5566);	//indicating that the os_ptr_ is ready to be access, 
														//otherwise, data race situation might occurs on os_ptr_ in parse function 
	                    	std::cerr<<"DONE CURL GET with total recv length of "<<got_length<<'\n'; 
						}
 					}
				) 
			) 
		);
		fut2_.wait ();
	}

/**
 * @brief main interface to parse, i.e. inflate and access, the data downloaded from get_from_url_impl function for downstream operation. \n A buffer length manage scheme is provided for the curl in buffer, a.k.a. userp, myos, os, os_ptr_ in the corresponding classes, in this parse function to cooperate with that provided in the curl_default_handle_mutex functor.
 */
//  while ( curl_put_ptr - curl_get_ptr < second_buf_size ), deteremine whether threading function get_from_url_impl has proceeded to an end by checking the fut_'s status; \n if not, wait till the ceriterion met (put_ptr-get_ptr >= second_buf_size), i.e. os_ptr_ has been filled with enough data for parsing, and proceed on normal parse operation; \n if so, proceed to terminal parse operation.
	void parse (std::stringstream& stri)
	{
		int curl_g=0, curl_p=0, gz_g=0, gz_p=0;
		{//inflate_result_stream_ << buf_element -> str();
			std::lock_guard<std::mutex> l(QQQ);
			curl_g = os_ptr_.get()->tellg (), curl_p = os_ptr_.get()->tellp ();
			gz_g = inflate_result_stream_.tellg (), gz_p = inflate_result_stream_.tellp ();
		}
		if ( gz_p-gz_g > second_buf_size*2)
		{//direct access data from inflate_result_stream_ whenever enough amount of inflate data are inflated and ready in the inflate_result_stream_
			parse_impl (stri);
			return;
		}
		while ( curl_p - curl_g < second_buf_size )
		{//wait scheme to guarantee the curl in buffer has enough amount of data for gz inflation before the download opeartion has proceeded to an end
			if ( fut_.wait_for (std::chrono::microseconds(100)) == std::future_status::timeout )
			{
				std::chrono::milliseconds dura ( 100 );
				std::this_thread::sleep_for ( dura );
				{
					std::lock_guard<std::mutex> ll(QQQ);
					curl_p = os_ptr_.get()->tellp ();
					curl_g = os_ptr_.get()->tellg ();
				}
			}
			else if ( fut_.wait_for (std::chrono::microseconds(100)) == std::future_status::ready )
			{
				for ( auto& Q : curl_in_thread_vec_)
					Q.join();
				terminate_flag_=true;
				break;
			}
		}
		if (!terminate_flag_)
		{//normal parse process, to copy data from os_ptr_ into inflate_result_stream_ for gz inflation
			char* k1 = new char [second_buf_size +1 ];
			{
				std::lock_guard<std::mutex> ll(QQQ);
				os_ptr_.get() -> read (k1, second_buf_size);
			}
			inflate_result_stream_.write ( k1, second_buf_size );
			delete [] k1;
			parse_impl (stri);
			got_size_ += second_buf_size;
		}
		else 
		{//terminal parse process, to access through the end of the os_ptr_
			int iii = os_ptr_.get()->tellp()-os_ptr_.get()->tellg();
			char* kk = new char [iii+1];
			os_ptr_.get() -> read (kk, iii);
			inflate_result_stream_.write ( kk, iii );
			delete [] kk;
			parse_impl_end (stri);
			terminate_flag_ = true;
			got_size_ += iii;
		}
	}

private:
/**
 * @brief implementaiton for estimate the potential inflated file length of the curl data 
 */
	void set_file_length (void)
	{
		CurlRecvImpl<&QQQ> QQ;
		auto gg = QQ.recv_impl ( this -> url_, this -> get_gz_file_size_impl () );
		gz_device_pipeline_.push ( *( QQ.get_result() ) );
		residue_ = *( (int*)( QQ.get_result() ) -> str().c_str() );
		uint64_t yy=2, yyy=yy<<32;
		file_length_ = (uint64_t) ( (uint64_t)( this -> gz_file_size_ * 3 / yyy) * yyy + (uint64_t)residue_ );
		gz_device_pipeline_.pop();
	}

/**
 * @brief implementaiton for obtaining a residue length, given by the potential inflated file length of the curl data % 4Gb, by parsing the last 4 bytes of the curl data
 */
	std::string get_gz_file_size_impl (void)
	{
		std::stringstream Qs;
		Qs << "Range: bytes="<<gz_file_size_-4 << "-" << gz_file_size_;
		return Qs.str();
	}

/**
 * @brief implementaiton for normal parse process
 */
	void parse_impl (std::stringstream& stri)
	{
		//copy data from inflate_result_stream_ into stri
		const int bitsize = second_buf_size*1;//.9;
		std::streamsize result;
		char* k = new char [bitsize+1];
		result = boost::iostreams::read ( gz_device_pipeline_, k, bitsize );
		k[result]='\0';
		stri.write (k, result);
		delete []k;

		//inflate_result_stream_ cleanup
        std::streamsize buf_p = inflate_result_stream_.tellp();
        std::streamsize buf_g = inflate_result_stream_.tellg();
        char* ka = new char [buf_p-buf_g];
        inflate_result_stream_.read (ka, buf_p-buf_g);
        inflate_result_stream_.str("");
        inflate_result_stream_.clear();
        inflate_result_stream_.write(ka, buf_p-buf_g);
        delete [] ka;

		//stri cleanup
		stri.seekp (0, std::ios::end);
		int stri_p = stri.tellp();
		int stri_g = stri.tellg();
		char* kkk = new char [stri_p-stri_g];
		stri.read ( kkk, stri_p-stri_g );
		stri.str ("");
		stri.clear();
		stri.write ( kkk, stri_p-stri_g );
		delete [] kkk;
	}

/**
 * @brief implementaiton for terminal parse process
 */
	void parse_impl_end (std::stringstream& stri)
	{
		std::streamsize bitsize = 1048576;
		char* k = new char [bitsize+1];
		std::streamsize result;
		while (true)
		{
			result = boost::iostreams::read ( gz_device_pipeline_, k, bitsize );
			if ( result == -1 )
				break;
			k[result]='\0';
			std::streamsize read_pos = inflate_result_stream_.tellg();
			inflate_result_stream_.str( inflate_result_stream_.str().substr (read_pos) );
			inflate_result_stream_.seekg (0);
			stri << k;
		}
		delete []k;
	}
};

/**
 * @class CurlWrapper <CompressFormat::PLAIN, FUNC>
 * @brief Specialized version of class CurlWrapper exclusively for the situation that the curl data are in plain format.
 */
template < typename FUNC >
class CurlWrapper < CompressFormat::PLAIN, FUNC >
{
public:
	std::string url_;
	size_t file_size_;
	std::shared_ptr<std::stringstream> os_ptr_;
	uint64_t file_length_; 
	std::deque <std::thread> curl_in_thread_vec_;
	std::future <size_t> fut_;
	std::future <size_t> fut2_;
	bool terminate_flag_;

public:
/**
 * @brief constructor 
 */
	CurlWrapper (void)
	{}

/**
 * @brief constructor 
 * @param url the to be curled url address
 * @param the file size of the curl data, counted in gz format
 */
	CurlWrapper ( const std::string& url, size_t gz_file_size = 0)
		: url_ (url)
		, file_size_ (gz_file_size)
		, os_ptr_()
		, file_length_ ( gz_file_size )
		, curl_in_thread_vec_ (0)
		, terminate_flag_ (false)
	{}

/**
 * @brief main interface to access the data downloaded from get_from_url_impl function for downstream operation. Similar buffer management scheme shown in the specialization of gz format is used here.
 */
	void parse (std::stringstream& stri)
	{
		int curl_g=0, curl_p=0;
		{//inflate_result_stream_ << buf_element -> str();
			std::lock_guard<std::mutex> l(QQQ);
			curl_g = os_ptr_.get()->tellg ();
			curl_p = os_ptr_.get()->tellp ();
		}
		while ( curl_p - curl_g < second_buf_size )
		{
			if ( fut_.wait_for (std::chrono::microseconds(100)) == std::future_status::timeout )
			{
				std::chrono::milliseconds dura ( 100 );
				std::this_thread::sleep_for ( dura );
				{
					std::lock_guard<std::mutex> ll(QQQ);
					curl_p = os_ptr_.get()->tellp ();
					curl_g = os_ptr_.get()->tellg ();
				}
			}
			else if ( fut_.wait_for (std::chrono::microseconds(100)) == std::future_status::ready )
			{
				for ( auto& Q : curl_in_thread_vec_)
					Q.join();
				terminate_flag_=true;
				break;
			}
		}
		if (!terminate_flag_)
		{
			char* k1 = new char [second_buf_size +1 ];
			{
				std::lock_guard<std::mutex> ll(QQQ);
				os_ptr_.get() -> read (k1, second_buf_size);
			}
			stri.write ( k1, second_buf_size );
			delete [] k1;
		}
		else 
		{
			int iii = os_ptr_.get()->tellp()-os_ptr_.get()->tellg();
			char* kk = new char [iii+1];
			os_ptr_.get() -> read (kk, iii);
			delete [] kk;
			stri.write ( kk, iii );
			terminate_flag_ = true;
		}
	}

/**
 * @brief main interface to start the curl download operation.  Similar buffer management scheme shown in the specialization of gz format is used here.
 */
	void get_from_url_impl (void)
	{
		std::shared_ptr < std::promise <size_t> > sptr ( new ( std::promise <size_t> ) );
		fut_ = sptr->get_future();
		std::shared_ptr < std::promise <size_t> > sptr2 ( new ( std::promise <size_t> ) );
		fut2_ = sptr2->get_future();
		curl_in_thread_vec_.push_back (
			std::move (
				std::thread (
					[this, sptr, sptr2] ()
					{
						uint64_t got_length (0);
						CurlRecvImpl <&QQQ> QQ;
						os_ptr_ = QQ.get_result();
						sptr2 -> set_value (5566);	
						got_length+=QQ.recv_impl ( this -> url_, "");//get in part not supported here
						sptr -> set_value (5566);	
	                    std::cerr<<"DONE CURL GET with total recv length of "<<got_length<<'\n'; 
					} 
				) 
			) 
		);
		fut2_.wait ();
	}
};

#endif
