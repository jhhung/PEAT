#include "../src/trimmer/single_end_adapter_trimmer.hpp"
#include "../src/file_reader.hpp"
#include "gtest/gtest.h"

typedef std::tuple<std::string, std::string, std::string, std::string> TUPLETYPE;
std::map<int, std::vector< Fastq<TUPLETYPE> > > ccc;//, cca;
std::vector<int> trim_pos;

TEST (single_end_adapter_trimmer, read1)
{
	std::vector<std::string> read_vec ({"/mnt/godzilla/johnny/PEAT/million_0/origin0_1.fq", "/mnt/godzilla/johnny/PEAT/million_0/origin0_1.fq"});//({"test.fq", "test.fq"});
	FileReader < ParallelTypes::M_T, Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE >
		QQA (read_vec, &ccc);
	QQA.Read (100000);
}

TEST (single_end_adapter_trimmer, Trim_MT)
{
	ParameterTrait <QualityScoreType::SANGER> Qoo ("AGATCGGAAGAGCGG");//("ATCGACGT");
	SingleEndAdapterTrimmer < ParallelTypes::M_T, Fastq, TUPLETYPE, QualityScoreType::SANGER > QQ (Qoo);//(adapter_seq);////("ATCGACGT");
//	QQ.Trim (&ccc, 1, trim_pos);	//18.20s
	QQ.Trim (&ccc, 2, trim_pos);	//10.40s
//	QQ.Trim (&ccc, 4, trim_pos);	//10.48s
//	QQ.Trim (&ccc, 8, trim_pos);	//10.18s
}

TEST (single_end_adapter_trimmer, MT_verify)
{
	std::cerr<<"MT_verify"<<'\n';
	//std::cerr<<"ccc[0].size [1].size trim_pos.size "<<ccc[0].size()<<'\t'<<ccc[1].size()<<'\t'<<trim_pos.size()<<std::endl;

	for ( auto jj=0; jj!=ccc[0].size(); ++jj)
	{
		if (trim_pos[jj]!=0)
		{
			std::get<1>(ccc[1][jj].data).resize (trim_pos[jj]);
			std::get<3>(ccc[1][jj].data).resize (trim_pos[jj]);
		}
//std::cerr<<"ccc[0].size [1].size trim_pos.size "<<std::get<1>(ccc[0][jj].data).size()<<'\t'<<std::get<1>(ccc[1][jj].data).size()<<'\t'<<trim_pos[jj]<<std::endl;
		EXPECT_EQ ( std::get<0>(ccc[0][jj].data), std::get<0>(ccc[1][jj].data) ); 
		EXPECT_EQ ( std::get<1>(ccc[0][jj].data), std::get<1>(ccc[1][jj].data) ); 
		EXPECT_EQ ( std::get<2>(ccc[0][jj].data), std::get<2>(ccc[1][jj].data) ); 
		EXPECT_EQ ( std::get<3>(ccc[0][jj].data), std::get<3>(ccc[1][jj].data) ); 
	}
	trim_pos.clear();
	ccc.clear();
}

TEST (single_end_adapter_trimmer, read2)
{
	std::vector<std::string> read_vec ({"/mnt/godzilla/johnny/PEAT/million_0/origin0_1.fq", "/mnt/godzilla/johnny/PEAT/million_0/origin0_1.fq"});//({"test.fq", "test.fq"});
	FileReader < ParallelTypes::M_T, Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE >
		QQA (read_vec, &ccc);
	QQA.Read (100000);
}

TEST (single_end_adapter_trimmer, Trim_NORMAL)
{
	ParameterTrait <QualityScoreType::SANGER> Qoo ("AGATCGGAAGAGCGG");//("ATCGACGT");
	SingleEndAdapterTrimmer < ParallelTypes::NORMAL, Fastq, TUPLETYPE, QualityScoreType::SANGER > QQ (Qoo);//(adapter_seq);////("ATCGACGT");
	QQ.Trim (&ccc, trim_pos);	//15.10s
}

TEST (single_end_adapter_trimmer, NORMAL_verify)
{
	std::cerr<<"MT_verify"<<'\n';
	//std::cerr<<"ccc[0].size [1].size trim_pos.size "<<ccc[0].size()<<'\t'<<ccc[1].size()<<'\t'<<trim_pos.size()<<std::endl;

	for ( auto jj=0; jj!=ccc[0].size(); ++jj)
	{
		if (trim_pos[jj]!=0)
		{
			std::get<1>(ccc[1][jj].data).resize (trim_pos[jj]);
			std::get<3>(ccc[1][jj].data).resize (trim_pos[jj]);
		}
//std::cerr<<"ccc[0].size [1].size trim_pos.size "<<std::get<1>(ccc[0][jj].data).size()<<'\t'<<std::get<1>(ccc[1][jj].data).size()<<'\t'<<trim_pos[jj]<<std::endl;
		EXPECT_EQ ( std::get<0>(ccc[0][jj].data), std::get<0>(ccc[1][jj].data) ); 
		EXPECT_EQ ( std::get<1>(ccc[0][jj].data), std::get<1>(ccc[1][jj].data) ); 
		EXPECT_EQ ( std::get<2>(ccc[0][jj].data), std::get<2>(ccc[1][jj].data) ); 
		EXPECT_EQ ( std::get<3>(ccc[0][jj].data), std::get<3>(ccc[1][jj].data) ); 
	}
	trim_pos.clear();
	ccc.clear();
}

