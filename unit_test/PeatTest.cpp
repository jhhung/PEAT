#include "../pokemon_public/src/PeatFree.hpp"

CURLcode report_failure (const std::string& urlin, const std::string& token)
{
	std::stringstream ss;
	CURLcode code (CURLE_FAILED_INIT);
	CURL* curl = curl_easy_init();
	curl_slist * pHeaders = NULL;

	ss << (urlin + token.substr (16) );
	std::string response_url (ss.str());
	ss.str("");

	if (curl)
	{
		if (	   CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_URL, response_url.c_str() ) ) 
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POST, 1L) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1L) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, 0) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, NULL) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, call_back_operate_mutexless <curl_default_handle> ) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &(ss) ) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, pHeaders) ) 

				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYHOST, 0 ) )
				&& CURLE_OK == ( code = curl_easy_setopt ( curl, CURLOPT_SSL_VERIFYPEER, 0 ) )
		   )
			code = curl_easy_perform (curl);
		curl_easy_cleanup (curl);
	}
	auto return_str = ss.str();
	std::cerr<<"report failure "<<return_str<<'\n';//json_recv_1_<<'\n';
	return code;
};


int main (int argc, char* argv[])
{
	typedef std::tuple<std::string, std::string, std::string, std::string> TUPLETYPE;
	json_handler<> jh (argv[1], "");
	if (jh.token_ == "0fcaae9e66f94bd087fadca32660e1e4")//"a45b59da3cf74fc7a104c5fae193b991")
	{
std::cerr<<"localhost 10g "<<std::endl;
	jh.url_vec_[0] = {"http://localhost/test/10g-1.fastq.gz", "http://localhost/test/10g-2.fastq.gz"};
	jh.url_vec_[1] = {"http://localhost/test/10g-3.fastq.gz", "http://localhost/test/10g-4.fastq.gz"};
	}
	if (jh.token_ == "bfb36a45cfa747d88b9b5d933c7c0259")
	{
std::cerr<<"localhost 400m "<<std::endl;
	jh.url_vec_[0] = {"http://localhost/test/400m-1.fastq.gz", "http://localhost/test/400m-2.fastq.gz"};
	jh.url_vec_[1] = {"http://localhost/test/400m-3.fastq.gz", "http://localhost/test/400m-4.fastq.gz"};
	}
	jh.print();

	try
	{
		for (auto index=0; index!=jh.url_vec_.size(); ++index)
		{
			std::cerr<<"init run # "<<index<<'\n';
			std::vector <std::string> upload_vec0 ({
					jh.upload_url_vec_[index][0]//"https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=text.txt&multipart=true",
					,jh.access_token_//"x-access-token: 4b2bf2c94ea34721ac01fc177111707f",
					, "Content-Type: application/gz"//, "Content-Type: application/binary"//text"
					, jh.basic_str_ //, "https://api.basespace.illumina.com/"
					});

			std::vector <std::string> upload_vec1 ({
					jh.upload_url_vec_[index][1]//"https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=text.txt&multipart=true",
					,jh.access_token_//"x-access-token: 4b2bf2c94ea34721ac01fc177111707f",
					, "Content-Type: application/gz"//, "Content-Type: application/binary"//text"
					, jh.basic_str_ //, "https://api.basespace.illumina.com/"
					});

			std::map<int, std::vector< Fastq<TUPLETYPE> > > ccc;
			std::map<int, std::vector< size_t > > ttt;

			size_t startsize_in;
			std::stringstream ss (argv[2]);
			double m_indicator, a_mismatch, g_mismatch;
			char comma1, comma2, comma3;
			ss >> m_indicator >> comma1 >> a_mismatch >> comma2 >> g_mismatch >> comma3 >> startsize_in;

			ParameterTrait i_parameter_trait ( startsize_in, 100000, 8);

			PEAT_CLOUD_MT < FileReaderComponentMT, PeatComponentMT	> QQ 
				( jh.url_vec_[index],   jh.size_vec_[index], 
				  upload_vec0, upload_vec1,
				  &ccc, &ttt, 
				  i_parameter_trait, m_indicator, a_mismatch, g_mismatch);
			size_t sum = 0, rd_count = 0;
			int i = 0;

			while (true)//(i<2)// (true)
			{
				std::cout<<"round # "<<i<<'\n';
				if (!QQ.Parse_N_Trim_all ("www.jhhlab.tw/basespace/query_task_peat/update_progress/", jh.uid_vec_[index]) )
					break;
				++i;
			}
			std::cerr<<"done PNTA # "<<index<<std::endl;
			if ( index==jh.url_vec_.size()-1 )
				;//QQ.terminate ("http://www.jhhlab.tw/basespace/query_task_peat/status_finish/");
		}
		std::cerr<<"done of all "<<std::endl;
	}

	catch (...)
	{
    	report_failure ("http://www.jhhlab.tw/basespace/query_task_peat/failure/", jh.access_token_);
		std::cerr<<"ERROR CATCHED"<<std::endl;
	}
}

/*	Non-M_T version
int main (int argc, char* argv[])
{
	typedef std::tuple<std::string, std::string, std::string, std::string> TUPLETYPE;
	json_handler<curl_default_handle> jh (argv[1], "test_1_0703");
	jh.print();
	auto index=0;
	{
		std::cerr<<"init run # "<<index<<'\n';
		std::vector <std::string> upload_vec0 ({
				jh.upload_url_vec_[index][0]//"https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=text.txt&multipart=true",
				,jh.access_token_//"x-access-token: 4b2bf2c94ea34721ac01fc177111707f",
				, "Content-Type: application/gz"//, "Content-Type: application/binary"//text"
				, "https://api.basespace.illumina.com/"
				});

		std::vector <std::string> upload_vec1 ({
				jh.upload_url_vec_[index][1]//"https://api.basespace.illumina.com/v1pre3/appresults/1650649/files?name=text.txt&multipart=true",
				,jh.access_token_//"x-access-token: 4b2bf2c94ea34721ac01fc177111707f",
				, "Content-Type: application/gz"//, "Content-Type: application/binary"//text"
				, "https://api.basespace.illumina.com/"
				});

		std::map<int, std::vector< Fastq<TUPLETYPE> > > ccc;
		std::map<int, std::vector< size_t > > ttt;

		size_t startsize_in;
		std::stringstream ss (argv[2]);
		double m_indicator, a_mismatch, g_mismatch;
		char comma1, comma2, comma3;
		ss >> m_indicator >> comma1 >> a_mismatch >> comma2 >> g_mismatch >> comma3 >> startsize_in;

		ParameterTrait i_parameter_trait ( startsize_in );//, num_in, pool_size_in );

		PEAT_CLOUD < FileReaderComponent, PeatComponent	> QQ 
				(  //vec1, vec2, 
				  jh.url_vec_[index],   jh.size_vec_[index], //file_vec, 
				  upload_vec0, upload_vec1,
				  &ccc, &ttt, //RQ, WQ, 
				  i_parameter_trait, m_indicator, a_mismatch, g_mismatch);
		size_t sum = 0, rd_count = 0;
		int i = 0;

		std::cerr<<"current buf size "<<QQ.Curl_device_[0]->thread_recv_volume_<<'\t'<<QQ.Curl_device_[1]->thread_recv_volume_<<std::endl;
		while (true)//(i<2)// (true)
		{
			std::cout<<"round # "<<i<<'\n';
			if (!QQ.Parse_N_Trim_all (3))
				break;
			++i;
		}
		std::cerr<<"done PNTA # "<<index<<std::endl;
		if ( index==jh.url_vec_.size() )
			std::cerr<<"fake termination "<<'\n';
		//	  QQ.terminate ("http://www.jhhlab.tw/basespace/query_task_PEAT_CLOUD/status_finish/");
	}
	std::cerr<<"done of all "<<std::endl;
}																																									  
*/
