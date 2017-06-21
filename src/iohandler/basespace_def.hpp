#ifndef BASESPACE_DEF_HPP_
#define BASESPACE_DEF_HPP_

#include "iohandler.hpp"
typedef boost::mpl::map
<
      boost::mpl::pair< IoHandlerGlobalParameter::FilteringStreamType, boost::iostreams::filtering_streambuf<boost::iostreams::output> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceParameter, DeviceParameter >
    , boost::mpl::pair< IoHandlerGlobalParameter::MutipleNumber, boost::mpl::int_<2> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceBufferSize, boost::mpl::int_<8> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DevicePushbackSize, boost::mpl::int_<16> >
    , boost::mpl::pair< IoHandlerGlobalParameter::BaseStreamType, std::ostream >

> FILE_WRITTER_BASESPACE_GLOBAL_SETTING;

typedef boost::mpl::vector
<   
    boost::mpl::map
    <  
        boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::BasespaceDevice> >,
        boost::mpl::pair< BaseSpaceLocalParameter::CurlSendMaxLength, boost::mpl::int_<6*1024*1024> >,
        boost::mpl::pair< BaseSpaceLocalParameter::CurlSendLastLength, boost::mpl::int_<5*1024*1024> >
    >
> FILE_WRITTER_BASESPACE_LIST;

typedef iohandler<FILE_WRITTER_BASESPACE_GLOBAL_SETTING, FILE_WRITTER_BASESPACE_LIST> IoHandlerBaseSpace;

typedef boost::mpl::map
<
    boost::mpl::pair< IoHandlerGlobalParameter::FilteringStreamType, boost::iostreams::filtering_streambuf<boost::iostreams::input> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceParameter, DeviceParameter >
    , boost::mpl::pair< IoHandlerGlobalParameter::MutipleNumber, boost::mpl::int_<2> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceBufferSize, boost::mpl::int_<8> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DevicePushbackSize, boost::mpl::int_<16> >
    , boost::mpl::pair< IoHandlerGlobalParameter::BaseStreamType, std::istream >

> FILE_WRITTER_BASESPACE_DOWNLOAD_GLOBAL_SETTING;

typedef boost::mpl::vector
<   
    boost::mpl::map
    <   
        boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::GzipDeCompressFilter> >
    >,

    boost::mpl::map
    <  
        boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::BasespaceDevice_download> >
    >
> FILE_WRITTER_BASESPACE_DOWNLOAD_LIST;

typedef iohandler<FILE_WRITTER_BASESPACE_DOWNLOAD_GLOBAL_SETTING, FILE_WRITTER_BASESPACE_DOWNLOAD_LIST> IoHandlerBaseSpaceDownload;


// local file ofstream
typedef boost::mpl::map
<
      boost::mpl::pair< IoHandlerGlobalParameter::FilteringStreamType, boost::iostreams::filtering_streambuf<boost::iostreams::output> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceParameter, DeviceParameter >
    , boost::mpl::pair< IoHandlerGlobalParameter::MutipleNumber, boost::mpl::int_<2> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceBufferSize, boost::mpl::int_<8> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DevicePushbackSize, boost::mpl::int_<16> >
    , boost::mpl::pair< IoHandlerGlobalParameter::BaseStreamType, std::ostream >

> FILE_WRITTER_OFSTREAM_GLOBAL_SETTING;

typedef boost::mpl::vector
<
    boost::mpl::map
    <
        boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::Ofstream> >
    >
> FILE_WRITTER_OFSTREAM_LIST;

typedef iohandler<FILE_WRITTER_OFSTREAM_GLOBAL_SETTING, FILE_WRITTER_OFSTREAM_LIST> IoHandlerOfstream;

// local file ifstream

typedef boost::mpl::map
<
      boost::mpl::pair< IoHandlerGlobalParameter::FilteringStreamType, boost::iostreams::filtering_streambuf<boost::iostreams::input> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceParameter, DeviceParameter >
    , boost::mpl::pair< IoHandlerGlobalParameter::MutipleNumber, boost::mpl::int_<2> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DeviceBufferSize, boost::mpl::int_<8> >
    , boost::mpl::pair< IoHandlerGlobalParameter::DevicePushbackSize, boost::mpl::int_<16> >
    , boost::mpl::pair< IoHandlerGlobalParameter::BaseStreamType, std::istream >

> FILE_WRITTER_IFSTREAM_GLOBAL_SETTING;

typedef boost::mpl::vector
<
    boost::mpl::map
    <
        boost::mpl::pair< IoHandlerGlobalParameter::DeviceType, boost::mpl::int_<FileDeviceType::Ifstream> >
    >
> FILE_WRITTER_IFSTREAM_LIST;

typedef iohandler<FILE_WRITTER_IFSTREAM_GLOBAL_SETTING, FILE_WRITTER_IFSTREAM_LIST> IoHandlerIfstream;

typedef boost::mpl::vector<
    boost::mpl::map<   
        boost::mpl::pair< 
			IoHandlerGlobalParameter::DeviceType, 
			boost::mpl::int_<FileDeviceType::GzipDeCompressFilter> 
		>
    >,
    boost::mpl::map<
        boost::mpl::pair< 
			IoHandlerGlobalParameter::DeviceType, 
			boost::mpl::int_<FileDeviceType::Ifstream> 
		>
	>

> FILE_IFSTREAM_LIST;
typedef iohandler<FILE_WRITTER_IFSTREAM_GLOBAL_SETTING, FILE_IFSTREAM_LIST> IoHandlerGzipIfstream;
#endif
