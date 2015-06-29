#ifndef FILE_GENERATOR_HPP_
#define FILE_GENERATOR_HPP_

#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include "boost/filesystem.hpp"

#include "boost/lexical_cast.hpp"
#include "../../src/constant_def.hpp"
#include "../../src/tuple_utility.hpp"
#include "../../src/wrapper_tuple_utility.hpp"
//#include "../../src/format/fasta.hpp"
#include "../../src/format/fastq.hpp"
//#include "../../src/format/bed.hpp"
//#include "../../src/format/wig.hpp"

class RandomFileGenerator
{
public:
	std::mt19937 gen;
	boost::filesystem::path temp_file_path;
	std::string temp_file_name;
	std::ofstream ofile;
	std::ifstream ifile;
//	std::random_device rd;

	RandomFileGenerator () 
	: gen( ) 
	, temp_file_path(boost::filesystem::temp_directory_path())
	{
		std::cerr<<"constructing"<<std::endl;
		temp_file_name = temp_file_path.string() + "/temp_file_for_test";
		std::cerr<<temp_file_name<<std::endl;
	}
	RandomFileGenerator (std::string pathName)
    : gen( ) 
    , temp_file_path()
    {
        std::cerr<<"constructing"<<std::endl;
        temp_file_name = pathName;
        std::cerr<<temp_file_name<<std::endl;
    }

	template<typename FORMAT>
	void GenRandFile(void)
	{
		FORMAT random_content(std::move(GenerateFormat< FORMAT >()));
		//std::cerr << random_content << std::endl;
		WriteToFile(random_content, temp_file_name);
	};
	
	template<typename FORMAT>
    FORMAT GenRandFile(int n)
    {
        FORMAT random_content(std::move(GenerateFormat< FORMAT >()));
        //std::cerr << random_content << std::endl;
        WriteToFile(random_content, temp_file_name);
		return random_content;
    };


	template<class T>
	T GenerateFormat(void)
	{};

	template<class T>
	void WriteToFile(T& content, std::string& s)
	{
		ofile.open(temp_file_name);
		ofile << content;
		ofile.close();
		//std::cerr << "temp file = " << temp_file_name << std::endl;
	};

	std::string GetContent(int line_num)
	{
		ifile.open(temp_file_name);
		std::string line,result;
		std::vector<std::string> data;
		while(getline(ifile,line))
			data.push_back(line);
		ifile.close();

		result = data[line_num];
		return result;
	};

};

/*
template<>
Fasta<> RandomFileGenerator::GenerateFormat()
{	
	char base[5]={'A','T','C','G'};
	std::uniform_int_distribution<int> unif(0,3);
	Fasta<> target;
	std::string head, sequence;
	std::vector<std::string> split_temp;
		
	for(int i=0; i!=5; ++i)
	{
		head = "name" + boost::lexical_cast<std::string>(i);
		for(int n=0; n<300; ++n)
		{
			sequence += base[unif(gen)];
			//if((n+1)%40==0)
			//	sequence += "\n";
		}
		split_temp.push_back(head);
		split_temp.push_back(sequence);
		sequence = "";
	}	
	
	//for(int j=0; j!=split_temp.size(); ++j)
	//	std::cerr << split_temp[j] << std::endl;

	TupleUtility< std::tuple<std::string, std::string>, std::tuple_size<Fasta<>::TupleType>::value >::FillTuple( target.data, split_temp );
	return target;
};
*/
template<>
Fastq<> RandomFileGenerator::GenerateFormat()
{
	std::uniform_int_distribution<int> unif(65,90);

	Fastq<> target;
	std::string name1, name2, sequence, quality;
	std::vector<std::string> split_temp;
		
	name1 = "@name1";
	name2 = "+name2";
	for(int n=0; n<60; ++n)
	{
		sequence += char(unif(gen));
		quality += char(unif(gen));
	}
	split_temp.push_back(name1);
	split_temp.push_back(sequence);
	split_temp.push_back(name2);
	split_temp.push_back(quality);

	TupleUtility< std::tuple<std::string, std::string, std::string, std::string>, std::tuple_size<Fastq<>::TupleType>::value >::FillTuple( target.data, split_temp );
	return target;
};
/*
template<>
Bed<> RandomFileGenerator::GenerateFormat()
{
	Bed<> target;
	std::string chrom, chrom_start, chrom_end;
	typedef std::string STRING;
	std::vector< STRING > split_temp;
	std::uniform_int_distribution<int> unif(0,9);
		
	chrom = "chr7";
	for(int n=0; n<5; ++n)
	{
		chrom_start += boost::lexical_cast<STRING>( unif(gen) );
		chrom_end +=  boost::lexical_cast<STRING>( unif(gen) );
	}

	split_temp.push_back(chrom);
	split_temp.push_back(chrom_start);
	split_temp.push_back(chrom_end);

	TupleUtility< std::tuple<std::string, uint32_t, uint32_t>, std::tuple_size<Bed<>::TupleType>::value >::FillTuple( target.data, split_temp );
	return target;
};
*/
/*
template<>
Wig<> RandomFileGenerator::GenerateFormat()
{
	typedef std::tuple <uint32_t, double> MyType;
	typedef std::vector < Wrapper <MyType> > VectorType;
	typedef std::tuple<std::string, std::string, int, Wrapper <VectorType> > TupleType;
	Wig<> target;
	std::string track("track type=wiggle_0 name=variableStep description=variableStep format visibility=full autoScale=off viewLimits=0.0:25.0 color=50,150,255 rLineMark=11.76 yLineOnOff=on priority=10");
	std::string chrom("chr17");
	std::string span("123");
	std::string step_info("*49304701#10.13333");

	typedef std::string STRING;
	std::vector< STRING > split_temp;

//	std::uniform_int_distribution<int> unif(0,9);
		
//	chrom = "chr7";
//	for(int n=0; n<5; ++n)
//	{
//		chrom_start += boost::lexical_cast<STRING>( unif(gen) );
//		chrom_end +=  boost::lexical_cast<STRING>( unif(gen) );
//	}

	//span = boost::lexical_cast<STRING>(200);

	split_temp.push_back(track);
	split_temp.push_back(chrom);
	split_temp.push_back(span);
	split_temp.push_back(step_info);
	
	typedef std::tuple <uint32_t, double> MyType;
	typedef std::vector < Wrapper <MyType> > VectorType;
	TupleUtility< TupleType, std::tuple_size<Wig<>::TupleType>::value >::FillTuple( target.data, split_temp );
	return target;
};
*/
#endif
