#include <fstream>
#include <sstream>
#include <iterator>
#include "pair_end_adapter_trimmer.hpp"
//#include "../file_reader.hpp"
//#include "../iohandler/ihandler/IhandlerFactory.hpp"
#include "../iohandler/ihandler/IhandlerFactory.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include "../format/fastq.hpp"

namespace paired_end
{
	int main (int argc, char* argv[])
	{
		std::string usage = R"(
*********************************************************************************
+----------+
|Paired End|
+----------+

A software do paired-end adapter trimming operation.
It takes paired-end FastQ format input files (dual files), and reports adapter 
removed FastQ format output files (dual files).

>> PEAT paired
    Do paired-end adapter trimming operation with instruction like:
    bin/PEAT_linux paired -1 test_file/test_paired1.fq -2 test_file/test_paired2.fq 

*********************************************************************************
)";
		/** option variables **/
		std::string inputFile_1;
		std::string inputFile_2;
		std::string outputFile;
		std::string outputFile_1;
		std::string outputFile_2;
		int nthreads;
		int minLen;
		std::string reverseCompareMisRatio;
		std::string geneCompareMisRatio;
		std::string adapterCompareMisRatio;
		double rcRatio, geneRatio, adapterRatio;
		float threshold;
		bool qtrim_flag {false};
		std::string qualityType;
		bool verboses {false};
		bool compressed_flag {false};
		bool adapter_contexts_flag {false};

		boost::program_options::options_description opts {usage};

		try 
		{
			opts.add_options ()
				("help,h", "display this help message and exit")
				("input1,1", boost::program_options::value<std::string>(&inputFile_1)->required(), "The paired_1 input FastQ file (.fq) or Gzip compressed FASTQ file (.fq.gz).")
				("input2,2", boost::program_options::value<std::string>(&inputFile_2)->required(), "The paired_2 input FastQ file (.fq) or Gzip compressed FASTQ file (.fq.gz).")
				("output,o", boost::program_options::value<std::string>(&outputFile)->default_value(std::string {"stdout"}), "Prefix for Output file name, stdout by default. If you choose this option, you couldn't use --output_1 and --output_2")
				("output_1", boost::program_options::value<std::string>(&outputFile_1)->default_value(std::string {"stdout"}), "Prefix for Output file part1 name, stdout by default ")
				("output_2", boost::program_options::value<std::string>(&outputFile_2)->default_value(std::string {"stdout"}), "Prefix for Output file part2 name, stdout by default ")
				("thread,n", boost::program_options::value<int>(&nthreads)->default_value(1), "Number of thread to use; if the number is larger than the core available, it will be adjusted automatically")
				("len,l", boost::program_options::value<int>(&minLen)->default_value(30), "Minimum gene fragment length, i.e. the fragment length for reverse complement check, 30 bp by default")
				("reverse_mis_rate,r", boost::program_options::value<std::string>(&reverseCompareMisRatio)->default_value("0.4"), "Mismatch rate applied in first stage reverse complement scan, 0.4 by default")
				("gene_mis_rate,g", boost::program_options::value<std::string>(&geneCompareMisRatio)->default_value("0.6"), "Mismatch rate applied in second stage gene portion check, 0.6 by default")
				("adapter_mis_rate,a", boost::program_options::value<std::string>(&adapterCompareMisRatio)->default_value("0.4"), "Mismatch rate applied in second stage adapter portion check, 0.4 by default")
			    ("qtrim", "Quality trimmer; trim the last base of the reads until the mean quality value of the reads is larger than threshold")
				("quality,q", boost::program_options::value<std::string>(&qualityType), "The quality type. Type any one of the following quality type indicator: ILLUMINA, PHRED, SANGER, SOLEXA\nOnly for the option: --qtrim")
                ("threshold,t", boost::program_options::value<float>(&threshold), "The threshold (quality value) of the quality trimmer, 30.0 by default\nOnly for the option: --qtrim")
				("out_gzip", "Compress the FASTQ output to Gzip file. This option is required the option: -o or --output_1/--output_2")
				("verbose", "Output running process by stderr")
				("adapter_contexts", "Output adapter contexts within the top ten numbers in report.txt; if you use this option, the program becomes slower.")
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
				if ( !vm.count("quality") )
				{
					std::cerr << "Error: Need the option: quality";
					exit(1);
				}
			}

			/** check the verbose option**/
			if ( vm.count("verbose") )
				verboses = true;


			/** check the adapter_contexts**/

			if ( vm.count("adapter_contexts") )
				adapter_contexts_flag = true;
				

			/** check the threshold option**/
        	if ( vm.count("threshold") && !vm.count("qtrim") )
        	{
           	 	std::cerr << "Error: cannot use the option: thredshold because of the loss of the option: qtrim";
           	 	exit(1);
        	}

			/** check the quality option**/
        	if ( vm.count("quality") && !vm.count("qtrim") )
			{
				std::cerr << "Error: cannot use the option: quality because of the loss of the option: qtrim";
				exit(1);
			}
			else
				qualityType = "SANGER";
			/** check the out_gzip**/
			if ( vm.count("out_gzip") )
			{
				if ( outputFile != "stdout" || ( outputFile_1 != "stdout" && outputFile_1 != "stdout" ))
					compressed_flag = true;
				else
				{
					std::cerr << "Error: cannot use the --out_gzip: This option is requried the option: -o or --output_1/--output_2" << std::endl;
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
		if (!boost::filesystem::exists (inputFile_1) || !boost::filesystem::exists (inputFile_2)) {
			std::cerr << "Error: Input fastq file " << inputFile_1 << " or "<< inputFile_2 << "  does not exist! Please double check.\nExiting..." << std::endl;
			exit (1);
		}

		/** check output **/
		std::ostream* out{nullptr};
		std::ostream* out_2{nullptr};
		boost::iostreams::filtering_ostream* out_gzip{nullptr};
		boost::iostreams::filtering_ostream* out2_gzip{nullptr};
		if ( ( outputFile == "stdout" && outputFile_1 == "stdout" && outputFile_2 == "stdout" ) || ( outputFile_1 == "-" && outputFile_2 == "-" ) ) 
			out = &std::cout;
		else if ( outputFile != "stdout" && ( outputFile_1 != "stdout" || outputFile_2 != "stdout" ) )  
		{
			std::cerr << "Error: cannot creat ouput file.\nThe options --output or [ --output_1, output_2 ] only is allowed to excute one.\nExiting..." << std::endl;
			exit (1);
		}
		else if ( ( outputFile_1 != "stdout" && outputFile_2 == "stdout" ) || ( outputFile_1 == "stdout" && outputFile_2 != "stdout" ) )
		{
			std::cerr << "Error: cannot creat ouput file.\nThe options --output_1 and --output_2 have to be provied simultaneously.\nExiting..." << std::endl;
			exit (1);
		}
		else if ( outputFile != "stdout" )  
		{

			if (!compressed_flag)
			{
				out = new std::ofstream {outputFile+"_paired1.fq"};
				out_2 = new std::ofstream {outputFile+"_paired2.fq"};
			}

			else
			{
				out = new std::ofstream {outputFile+"_paired1.gz"};
				out_2 = new std::ofstream {outputFile+"_paired2.gz"};

				out_gzip = new boost::iostreams::filtering_ostream();
				out_gzip->push(boost::iostreams::gzip_compressor());
				out_gzip->push(*out); 
				out2_gzip = new boost::iostreams::filtering_ostream();
				out2_gzip->push(boost::iostreams::gzip_compressor());
				out2_gzip->push(*out_2); 
			}

			if (!*out || !*out_2) 
			{
				std::cerr << "Error: cannot creat output file " << outputFile+"_paired1 and " <<outputFile+"_paired2" << ".\nPlease double check.\nExiting..." << std::endl;
				exit (1);
			}
		}
		else
		{
			///不需要ogzip的判斷：名字由使用者自己取
			out = new std::ofstream {outputFile_1};
			out_2 = new std::ofstream {outputFile_2};
			
			if (!*out || !*out_2) 
			{
				std::cerr << "Error: cannot creat output file " << outputFile_1+" and " << outputFile_2 << ".\nPlease double check.\nExiting..." << std::endl;
				exit (1);
			}
		}

		/** check thread **/
/*
		auto nCore = boost::thread::hardware_concurrency();
		if ( nCore != 0 && nthreads > nCore) 
		{
			std::cerr << "Warning: the number of threads set (" << nthreads << ") is larger than the number of cores available (" << nCore << ") in this machine.\nSo reset -n=" << nCore << std::endl;
			nthreads = nCore;
		}
*/
		//**open the report.txt
		std::ostream* out_report{nullptr}; 
		std::string temp_str;
		if ( ( outputFile == "stdout" && outputFile_1 == "stdout" && outputFile_2 == "stdout" ) || ( outputFile_1 == "-" && outputFile_2 == "-" ) ) 
		{
			out_report = &std::cout;
		}
		else if ( outputFile != "stdout" && outputFile_1 == "stdout" )
		{
			temp_str = outputFile;
			std::vector<std::string> temp;
			boost::split ( temp, temp_str, boost::is_any_of ( "." ) );
			std::string ReportFile {temp[0] + "_report.txt" };
			out_report = new std::ofstream {ReportFile};
		}
		else
		{
			temp_str = outputFile_1;
			std::vector<std::string> temp;
			boost::split ( temp, temp_str, boost::is_any_of ( "." ) );
			std::string ReportFile {temp[0] + "_report.txt" };
			out_report = new std::ofstream {ReportFile};
		}

		typedef std::tuple<std::string, std::string, std::string, std::string> TUPLETYPE;
		std::vector<std::string> read_vec ({ inputFile_1, inputFile_2 });
		std::map<int, std::vector< Fastq<TUPLETYPE> > > result;
		std::map<int, std::vector< Fastq<TUPLETYPE> > > resultkk;
		rcRatio = boost::lexical_cast <double> (reverseCompareMisRatio);
		geneRatio = boost::lexical_cast <double> (geneCompareMisRatio);
		adapterRatio = boost::lexical_cast <double> (adapterCompareMisRatio);
//		extern IhandlerFactory<Fastq, TUPLETYPE> fastq_ihandler_factory;
		auto tup = IhandlerFactory<Fastq, TUPLETYPE>::get_ihandler_read (read_vec, result);
//		auto tup = FastqIhandler::get_ihandler_read (read_vec, result);
		auto ihandler 	= std::get<0>(tup);
		auto deletor 	= std::get<1>(tup);

		ParameterTrait <> i_parameter_trait (minLen, rcRatio, geneRatio, adapterRatio, 100000, nthreads, threshold, qualityType, adapter_contexts_flag);
		PairEndAdapterTrimmer <ParallelTypes::M_T, Fastq, TUPLETYPE, TrimTrait<std::string, LinearStrMatch <double>, double, double > > PEAT (i_parameter_trait);
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
			bool eof = ihandler(&result, 100000);
			PEAT.Sum (&result, sum_length_original, sum_reads_original);
			if (qtrim_flag) 
			{
				PEAT.QTrim(&result);
				PEAT.Sum (&result, sum_length_Q, sum_reads_Q);	
			}
			PEAT.Trim (&result, nthreads);
			PEAT.Sum (&result, sum_length, sum_reads);
			count_reads += result[0].size();
			PEAT.Verbose( verboses, count_reads, flag_type );

			if (out != &std::cout && !compressed_flag)
			{
				for (auto& Q : result.begin()->second)
				{
					(*out)<<Q;
				}
				auto itr = result.begin();
				std::advance (itr, 1);
				for (auto& Q : itr->second)
					(*out_2)<<Q;
			}
			else if (compressed_flag)
			{
				for (auto& Q : result.begin()->second)
				{
					(*out_gzip)<<Q;
				}
				auto itr = result.begin();
				std::advance (itr, 1);
				for (auto& Q : itr->second)
					(*out2_gzip)<<Q;
			}
			else //std::cout
			{
				for (auto& Q : result.begin()->second)
					(*out)<<Q;
				auto itr = result.begin();
				std::advance (itr, 1);
				for (auto& Q : itr->second)
					(*out)<<Q;
			}

			if (eof)
			{
				flag_type = 2;
				PEAT.Verbose( verboses, count_reads, flag_type );
				break;
			}
		}

		if (out != &std::cout)
		{
			if(compressed_flag)
			{
				boost::iostreams::close(*out_gzip);
				boost::iostreams::close(*out2_gzip);
				delete out_gzip, out2_gzip;
			}
			else;
			static_cast<std::ofstream*>(out)-> close();
			static_cast<std::ofstream*>(out_2)-> close();
			delete out, out_2;
		} 
		else;

		PEAT.Summary ( sum_length_original, sum_reads_original, 0, out_report );
		if (qtrim_flag) 
			PEAT.Summary ( sum_length_Q, sum_reads_Q, 1, out_report );
		else;
		PEAT.Summary ( sum_length, sum_reads, out_report, PEAT.adapter_context_set_ );
		if (out_report != &std::cout)
		{
			static_cast<std::ofstream*>(out_report)-> close ();
			delete out_report;
		}
		else;
		deletor();
		return 0;
	}
}
