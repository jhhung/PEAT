#include <fstream>
#include <iostream>
#include <string>
#include "../file_reader.hpp"
#include "single_end_adapter_trimmer_parameter.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
namespace single_end
{
	int main( int argc, char** argv )
	{
		std::string usage = R"(
*********************************************************************************
+----------+
|Single End|
+----------+

  A software using Naive Bayes classifier to do single-end adapter trimming operation.
  It takes FastQ format input file, and reports adapter removed FastQ format output file.
  
>> PEAT single
    Do single-end adapter trimming operation, with instruction like: 
	bin/PEAT_linux single -i test_file/test_paired1.fq -a AGATCGGAAGAGCG -q SANGER

*********************************************************************************
)";
		/** option variables **/
		std::string inputFile {};
		std::string adapterSeq {};
		std::string qualityType {};
		std::string outputFile {};
		int nthreads {};
		boost::program_options::options_description opts {usage};

		try 
		{
			opts.add_options ()
				("help,h", "display this help message and exit")
				("input,i", boost::program_options::value<std::string>(&inputFile)->required(), "The input FastQ file.")
				("adapter,a", boost::program_options::value<std::string>(&adapterSeq)->required(), "The adapter sequence, with minimum length of six characters.")
				("quality,q", boost::program_options::value<std::string>(&qualityType)->required(), "The quality type. Type any one of the following quality type indicator: ILLUMINA, PHRED, SANGER, SOLEXA")
				("output,o", boost::program_options::value<std::string>(&outputFile)->default_value(std::string {"stdout"}), "Output FastQ file, stdout by default ")			
				("thread,n", boost::program_options::value<int>(&nthreads)->default_value(1), "Number of thread to use; if the number is larger than the core available, it will be adjusted automatically")
				;
			boost::program_options::variables_map vm;
			boost::program_options::store (boost::program_options::parse_command_line(argc, argv, opts), vm);
			boost::program_options::notify(vm);
		} 
		catch (std::exception& e) 
		{
			std::cerr << "Error: " << e.what() << std::endl;
			std::cerr << opts << std::endl;
			exit (1);
		} 
		catch (...) 
		{
			std::cerr << "Unknown error!" << std::endl;
			std::cerr << opts << std::endl;
			exit (1);
		}

		/** check input fastq **/
		if (!boost::filesystem::exists (inputFile)) 
		{
			std::cerr << "Error: Input fastq file " << inputFile << " does not exist! Please double check.\nExiting..." << std::endl;
			exit (1);
		}

		/** check adapter sequence**/
		if (adapterSeq.size()<6) 
		{
			std::cerr << "Error: Input adapter sequence too short! Please provide adapter sequence with at least six character.\nExiting..." << std::endl;
			exit (1);
		}

		/** check output **/
		std::ostream* out{nullptr};
		if (outputFile == "stdout" || outputFile == "-") 
			out = &std::cout;
		else 
		{
			out = new std::ofstream {outputFile};
			if (!*out) 
			{
				std::cerr << "Error: cannot creat output file " << outputFile << ".\nPlease double check.\nExiting..." << std::endl;
				exit (1);
			}
		}

		/** check thread **/
		auto nCore = boost::thread::hardware_concurrency();
		if ( nCore != 0 && nthreads > nCore) 
		{
			std::cerr << "Warning: the number of threads set (" << nthreads << ") is larger than the number of cores available (" << nCore << ") in this machine.\nSo reset -n=" << nCore << std::endl;
			nthreads = nCore;
		}

		typedef std::tuple<std::string, std::string, std::string, std::string> TUPLETYPE;
		std::vector<std::string> read_vec ({inputFile});
		std::map<int, std::vector< Fastq<TUPLETYPE> > > result;
		FileReader < ParallelTypes::NORMAL, Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE > FileReader (read_vec, &result);

		ParameterTraitSeat parameter_trait (adapterSeq, qualityType, nthreads);
		SingleEndAdapterTrimmer <ParallelTypes::M_T, Fastq, TUPLETYPE> SEAT (parameter_trait);
		std::vector<int> trim_pos;

		while (true)	
		{
			bool eof = FileReader.Read (&result, 100000);
			SEAT.Trim (&result, nthreads, trim_pos);
			for (auto& Q : result.begin()->second)
				(*out)<<Q;
			if (eof)
				break;
		}

		if (out != &std::cout) 
		{
			static_cast<std::ofstream*>(out)-> close ();
			delete out;
		}
		return 0;
	}
}
