#include <fstream>
#include <iostream>
#include <string>
//#include "../file_reader.hpp"
#include "../iohandler/ihandler/IhandlerFactory.h"
#include "single_end_adapter_trimmer_parameter.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "../format/fastq.hpp"
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
  It takes FastQ/FastA format input file, and reports adapter removed FastQ/FastA format output file.
  
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
		bool qtrim_flag {false};
		float threshold;
		bool verboses {false};
		bool compressed_flag {false};
		boost::program_options::options_description opts {usage};

		try 
		{
			opts.add_options ()
				("help,h", "display this help message and exit")
				("input,i", boost::program_options::value<std::string>(&inputFile)->required(), "The input FastQ file (.fq) or Gzip compressed FASTQ file (.fq.gz).")
				("adapter,a", boost::program_options::value<std::string>(&adapterSeq)->required(), "The adapter sequence, with minimum length of six characters.")
				("quality,q", boost::program_options::value<std::string>(&qualityType)->default_value("ILLUMINA"), "The quality type. Type any one of the following quality type indicator: ILLUMINA, PHRED, SANGER, SOLEXA (default: ILLUMINA)")
				("output,o", boost::program_options::value<std::string>(&outputFile)->default_value(std::string {"stdout"}), "Output FastQ file, stdout by default ")			
				("thread,n", boost::program_options::value<int>(&nthreads)->default_value(1), "Number of thread to use; if the number is larger than the core available, it will be adjusted automatically")
				("qtrim", "Quality trimmer; trim the last base of the reads until the mean score is larger than threshold")
				("threshold,t", boost::program_options::value<float>(&threshold), "The threshold value of the quality trimmer, 30.0 by default")
				("out_gzip", "Compress the FASTQ output to Gzip file. This option is required the option: -o")
				("verbose", "Output running process by stderr ")
				;
			boost::program_options::variables_map vm;
			boost::program_options::store (boost::program_options::parse_command_line(argc, argv, opts), vm);
			boost::program_options::notify(vm);
			
			/** check the qtrim option**/
			if ( vm.count("qtrim") )
			{
				qtrim_flag = true;
				if ( !vm.count("threshold") )
                    threshold=30.0;
			}

			/** check the verbose option **/
			if ( vm.count("verbose") )
				verboses = true;
			
			/** check the threshold option**/
            if ( vm.count("threshold") && !vm.count("qtrim") )
            {   
                std::cerr << "Error: cannot use the option: thredshold because of the loss of the option: qtrim\n";
                exit(1);
            }   

			/** check the out_gzip**/
			if ( vm.count("out_gzip") )
			{
				if ( outputFile != "stdout" )
					compressed_flag = true;
				else
				{
					std::cerr << "Error: cannot use the --out_gzip: This option is requried the option: -o" << std::endl;
					exit(1);
				}
			}
			else;
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
		boost::iostreams::filtering_ostream* out_gzip{nullptr};
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
			else if(compressed_flag)
			{
				out_gzip = new boost::iostreams::filtering_ostream();	
				out_gzip->push(boost::iostreams::gzip_compressor());
				out_gzip->push(*out);
			}
			else;
		}

		/** check thread **/
//		auto nCore = boost::thread::hardware_concurrency();
//		if ( nCore != 0 && nthreads > nCore) 
//		{
//			std::cerr << "Warning: the number of threads set (" << nthreads << ") is larger than the number of cores available (" << nCore << ") in this machine.\nSo reset -n=" << nCore << std::endl;
//			nthreads = nCore;
//		}

		//**open the report.txt
		std::ostream* out_report{NULL}; 
		if (outputFile == "stdout" || outputFile == "-") 
			out_report = &std::cout;
		else
		{
			std::string temp_str {outputFile};
			std::vector<std::string> temp;
			boost::split ( temp, temp_str, boost::is_any_of ( "." ) );
			std::string ReportFile {temp[0] + "_report.txt" };
			out_report = new std::ofstream {ReportFile};
		}

		typedef std::tuple<std::string, std::string, std::string, std::string> TUPLETYPE;
		std::vector<std::string> read_vec ({inputFile});
		std::map<int, std::vector< Fastq<TUPLETYPE> > > result;
//		FileReader < ParallelTypes::NORMAL, Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE > FileReader (read_vec, &result);
		auto tup = IhandlerFactory<Fastq, TUPLETYPE>::get_ihandler_read (read_vec, result);
		auto ihandler 	= std::get<0>(tup);
		auto deletor 	= std::get<1>(tup);

		ParameterTraitSeat parameter_trait (adapterSeq, qualityType, nthreads, threshold);
		SingleEndAdapterTrimmer <ParallelTypes::M_T, Fastq, TUPLETYPE> SEAT (parameter_trait);
		std::vector<int> trim_pos;
		std::vector<int> Qtrim_pos;
		int Q_read_number;
		uint32_t count_reads(0);
        uint32_t sum_reads_original(0);
        uint32_t sum_length_original(0);
       	uint32_t sum_reads_Q(0);
        uint32_t sum_length_Q(0);
		uint32_t sum_reads(0);
		uint32_t sum_length(0);
		int flag_type (0);
		while (true)	
		{
//			bool eof = FileReader.Read (&result, 100000);
			bool eof = ihandler (&result, 100000);
			SEAT.Sum (&result, sum_length_original, sum_reads_original);
			if (qtrim_flag)
			{
				SEAT.QTrim (&result);
				SEAT.Sum (&result, sum_length_Q, sum_reads_Q);
			}
			SEAT.Trim (&result, nthreads, trim_pos);
			SEAT.Sum (&result, sum_length, sum_reads);
			if(!compressed_flag)
			{
				for (auto& Q : result.begin()->second)
					(*out)<<Q;
			}
			else
			{
				for (auto& Q : result.begin()->second)
					(*out_gzip)<<Q;
			}
			count_reads += result[0].size();
			SEAT.Verbose( verboses, count_reads, flag_type );
			if (eof)
			{
				flag_type = 2;
				SEAT.Verbose( verboses, count_reads, flag_type );
				break;
			}
		}

		if (out != &std::cout) 
		{
			if (compressed_flag)
			{
				boost::iostreams::close(*out_gzip);	 
				delete out_gzip;
			}
			static_cast<std::ofstream*>(out)->close();
			delete out;
		}
		SEAT.Summary ( sum_length_original, sum_reads_original, adapterSeq, 0, out_report );
		if (qtrim_flag)
			SEAT.Summary ( sum_length_Q, sum_reads_Q, adapterSeq, 1, out_report );
		SEAT.Summary ( sum_length, sum_reads, adapterSeq, 2, out_report );
		if (out_report != &std::cout) 
		{
			static_cast<std::ofstream*>(out_report)-> close ();
			delete out_report;
		}
		deletor();
		return 0;
	}
}
