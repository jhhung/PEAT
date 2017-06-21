#include <iostream>
#include "curl/curl.h"
#include <map>
#include <boost/type_traits.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/mpl/char.hpp>
#include <boost/mpl/string.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/any.hpp>
#include <boost/mpl/range_c.hpp>
//#include "curl_utility.hpp"
#include "curl_impl.hpp"
#include "gtest/gtest.h"

//enum CURLOPT
//{
//	URL = 1,
//	POST = 2
//};
//
//typedef boost::mpl::map
//<
//      boost::mpl::pair< boost::mpl::int_<CURLOPT::URL>, char* >
//    , boost::mpl::pair< boost::mpl::int_<CURLOPT::POST>, int >
//
//> CURL_SETTING; 
//
//
//template <typename SETTING = CURL_SETTING>
//struct QQ
//{   
//    typedef typename boost::mpl::at< SETTING, boost::mpl::int_<CURLOPT::URL> >::type URLType;
//    typedef typename boost::mpl::at< SETTING, boost::mpl::int_<CURLOPT::POST> >::type POSTType;
//};

/*
template <typename CURL_SETTING, int N>
struct Getter
{
	typedef boost::
	static void Get (std::map<int, void*>& paramemter)
	{
		Getter <N-1> :: Get (parameter);
		boost::mpl::if_ (bost::mpl::is_same <
	}
};

template <int >
struct Getter
{
	static void Get (std::map<int, void*>& paramemter)
	{
		Getter <N-1> :: Get (parameter);

	}
};
*/


//int main (void)
//{
///*
//	QQ <> CurlSettingDevice;
//	QQ<>::URLType A = "abce";
//	QQ<>::POSTType B = 5566;
//
////	std::cerr<< CINIT(URL, OBJECTPOINT, 2) <<'\n';
//
//	std::map<int, void*> QQ(
//	{
//	{1, (void*)("abcdefg")},
//	{2, (void*)(55)}
//	});
//*/
////	std::stringstream ss;
////	CURLcode code (CURLE_FAILED_INIT);
////	CURL* curl = curl_easy_init();
//	//curl_slist * pHeaders = NULL;
//
///*
//	typedef std::map < CURLoption, boost::any > CurlParameterType;
//	char* aa ("https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=ttf2.txt&multipart=true");//("abcdefgh");
//	curl_slist * pHeaders = NULL;
//	char* token ("x-access-token: 4b2bf2c94ea34721ac01fc177111707f");
//	char* content_type ("Content-Type: application/text");
//	pHeaders = curl_slist_append (pHeaders, token);//_.c_str());
//	pHeaders = curl_slist_append (pHeaders, content_type);//_.c_str());
//	CurlParameterType QQ
//	({ 
//	{CURLOPT_POST, 1L},
//	{CURLOPT_URL, aa},
//	{CURLOPT_VERBOSE, 1L},
//	{CURLOPT_POSTFIELDSIZE, 0L},
//	{CURLOPT_POSTFIELDS, (void*)NULL},
//	{CURLOPT_WRITEFUNCTION, &call_back_operate_mutexless <curl_default_handle>},
//	{CURLOPT_WRITEDATA, (&(ss))},
//	{CURLOPT_HTTPHEADER, pHeaders},
//	{CURLOPT_NOSIGNAL, 1L},
//	{CURLOPT_SSL_VERIFYHOST, 0L},
//	{CURLOPT_SSL_VERIFYPEER, 0L}
//	});
//	CurlImpl < CurlParameterType > YY (QQ);
//	YY.DoCurl();
//*/
//
//}

TEST (CurlImpl, ReportGetLength)
{
	CurlConfigSet <> QQQ;
	CurlImpl <> YYY(QQQ);
	std::stringstream sss;
	YYY.GetConfig ("https://api.basespace.illumina.com/v1pre3/files/535642/content?access_token=7b390a45253943ac83c047a0b9372ac7");
	YYY.HeaderConfig (std::vector<std::string>({"Range: bytes=0-"}));
	YYY.ExecuteCurl();//(5566);
	std::cerr<<"length "<<YYY.ReportGetLength()<<'\n';
//	std::cerr<<"Get result "<<YYY.GetWriteContent().str()<<'\n';
	YYY.CloseCurl();
};

/*
//int main (void)
TEST (CurlImpl, constructor)
{
	char* content ("ABCDEFG");
	curl_slist * pHeaders = NULL;
	pHeaders = curl_slist_append (pHeaders, "x-access-token: 4b2bf2c94ea34721ac01fc177111707f");
	pHeaders = curl_slist_append (pHeaders, "Content-Type: application/text");

	CurlConfigSet<> QQ ( std::map <CURLoption, boost::any>
	({
		{CURLOPT_VERBOSE, 1L},
		{CURLOPT_NOSIGNAL, 1L},
		{CURLOPT_SSL_VERIFYHOST, 0L},
		{CURLOPT_SSL_VERIFYPEER, 0L},
		{CURLOPT_POSTFIELDS, content},
		{CURLOPT_HTTPHEADER, pHeaders},
		{CURLOPT_CUSTOMREQUEST, (char*)"PUT"},
		{CURLOPT_URL, (char*)("https://api.basespace.illumina.com/v1pre3/files/117136460/parts/1")}
	}));
	CurlImpl <> YYY (QQ);//  (CurlConfigSet<> (custom_set) ) );
	
	YYY.WriteConfig (&YYY.default_buf_, call_back_operate_mutexless <curl_default_handle>);
	
	YYY.ExecuteCurl();
	std::cerr<<"Post result "<<YYY.GetWriteContent().str()<<'\n';
	YYY.CloseCurl();

}

TEST (CurlImpl, CustomConfig)
{
	CurlConfigSet <> QQQ;
	CurlImpl <> YYY(QQQ);
	char* content ("ABCDEFG");
	curl_slist * pHeaders = NULL;
	pHeaders = curl_slist_append (pHeaders, "x-access-token: 4b2bf2c94ea34721ac01fc177111707f");
	pHeaders = curl_slist_append (pHeaders, "Content-Type: application/text");

	std::map <CURLoption, boost::any> custom_set ({
	{CURLOPT_VERBOSE, 1L},
	{CURLOPT_NOSIGNAL, 1L},
	{CURLOPT_SSL_VERIFYHOST, 0L},
	{CURLOPT_SSL_VERIFYPEER, 0L},
	{CURLOPT_POSTFIELDS, content},
	{CURLOPT_HTTPHEADER, pHeaders},
	{CURLOPT_CUSTOMREQUEST, (char*)"PUT"},
	{CURLOPT_URL, (char*)("https://api.basespace.illumina.com/v1pre3/files/117136460/parts/1")},
	{CURLOPT_WRITEDATA, (void*)&YYY.default_buf_},
	{CURLOPT_WRITEFUNCTION, &call_back_operate_mutexless <curl_default_handle>}
	});
	YYY.CustomConfig (custom_set);
	YYY.ExecuteCurl();
	std::cerr<<"Post result "<<YYY.GetWriteContent().str()<<'\n';
	YYY.CloseCurl();
}

TEST (CurlImpl, PostConfig)
{
	CurlConfigSet <> QQQ;
	CurlImpl <> YYY (QQQ);
	YYY.PostConfig ("https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=ttf2.txt&multipart=true");
	YYY.HeaderConfig (std::vector<std::string>({"x-access-token: 4b2bf2c94ea34721ac01fc177111707f","Content-Type: application/text"}));

	YYY.ExecuteCurl();
	std::cerr<<"Post result "<<YYY.GetWriteContent().str()<<'\n';
	YYY.CloseCurl();
}

TEST (CurlImpl, WriteConfig)
{
	CurlConfigSet <> QQQ;
	CurlImpl <> YYY (QQQ);
	YYY.PostConfig ("https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=ttf2.txt&multipart=true");
	YYY.HeaderConfig (std::vector<std::string>({"x-access-token: 4b2bf2c94ea34721ac01fc177111707f","Content-Type: application/text"}));

	std::stringstream ss;
	YYY.WriteConfig(&ss, call_back_operate_mutexless <curl_default_handle>);

	YYY.ExecuteCurl();
	std::cerr<<"postconfig & writeconfig result: "<<ss.str()<<'\n';
	YYY.CloseCurl();
}

TEST (CurlImpl, PutConfig)
{
	CurlConfigSet <> QQQ;
	CurlImpl <> YYY(QQQ);
	char* content ("ABCDEFG");

	YYY.PutConfig("https://api.basespace.illumina.com/v1pre3/files/117136460/parts/1", content, 7 );
	YYY.HeaderConfig (std::vector<std::string>({"x-access-token: 4b2bf2c94ea34721ac01fc177111707f","Content-Type: application/text"}));

	YYY.ExecuteCurl();
	std::cerr<<"Put result "<<YYY.GetWriteContent().str()<<'\n';
	YYY.CloseCurl();
}

TEST (CurlImpl, GetConfig)
{
	CurlConfigSet <> QQQ;
	CurlImpl <> YYY(QQQ);
	std::stringstream sss;
	YYY.GetConfig ("https://api.basespace.illumina.com/v1pre3/files/535642/content?access_token=7b390a45253943ac83c047a0b9372ac7");
	YYY.HeaderConfig (std::vector<std::string>({"Range: bytes=0-5566"}));
	YYY.ExecuteCurl();
//	std::cerr<<"Get result "<<YYY.GetWriteContent().str()<<'\n';
	YYY.CloseCurl();

	YYY.GetConfig ("https://api.basespace.illumina.com/v1pre3/files/535642/content?access_token=7b390a45253943ac83c047a0b9372ac7");
	YYY.HeaderConfig (std::vector<std::string>({"Range: bytes=0-5566"}));
	std::stringstream ss;
	YYY.ChangeRecvBuf (&ss, call_back_operate_mutexless <curl_default_handle>);
	YYY.ExecuteCurl();
//	std::cerr<<"recv result "<<ss.str()<<'\n';
	EXPECT_EQ (ss.str(), YYY.GetWriteContent().str());
	YYY.CloseCurl();
};
*/


