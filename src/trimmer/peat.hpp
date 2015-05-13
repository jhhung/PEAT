#include <fstream>
#include <sstream>
#include <iterator>
#include "pair_end_adapter_trimmer.hpp"
#include "../file_reader.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

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
		bool verboses {false};

		boost::program_options::options_description opts {usage};

		try 
		{
			opts.add_options ()
				("help,h", "display this help message and exit")
				("input1,1", boost::program_options::value<std::string>(&inputFile_1)->required(), "The paired_1 input FastQ file.")
				("input2,2", boost::program_options::value<std::string>(&inputFile_2)->required(), "The paired_2 input FastQ file.")
				("output,o", boost::program_options::value<std::string>(&outputFile)->default_value(std::string {"stdout"}), "Prefix for Output file name, stdout by default. If you choose this option, you couldn't use --output_1 and --output_2")
				("output_1", boost::program_options::value<std::string>(&outputFile_1)->default_value(std::string {"stdout"}), "Prefix for Output file part1 name, stdout by default ")
				("output_2", boost::program_options::value<std::string>(&outputFile_2)->default_value(std::string {"stdout"}), "Prefix for Output file part2 name, stdout by default ")
				("thread,n", boost::program_options::value<int>(&nthreads)->default_value(1), "Number of thread to use; if the number is larger than the core available, it will be adjusted automatically")
				("len,l", boost::program_options::value<int>(&minLen)->default_value(30), "Minimum gene fragment length, i.e. the fragment length for reverse complement check, 30 bp by default")
				("reverse_mis_rate,r", boost::program_options::value<std::string>(&reverseCompareMisRatio)->default_value("0.4"), "Mismatch rate applied in first stage reverse complement scan, 0.4 by default")
				("gene_mis_rate,g", boost::program_options::value<std::string>(&geneCompareMisRatio)->default_value("0.6"), "Mismatch rate applied in second stage gene portion check, 0.6 by default")
				("adapter_mis_rate,a", boost::program_options::value<std::string>(&adapterCompareMisRatio)->default_value("0.4"), "Mismatch rate applied in second stage adapter portion check, 0.4 by default")
				("verbose", "Output running process by stderr ")
				;
			boost::program_options::variables_map vm;
			boost::program_options::store (boost::program_options::parse_command_line(argc, argv, opts), vm);
			boost::program_options::notify(vm);

			/** check the verbose option **/
			if ( vm.count("verbose") )
				verboses = true;
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
			out = new std::ofstream {outputFile+"_paired1.fq"};
			out_2 = new std::ofstream {outputFile+"_paired2.fq"};
			if (!*out || !*out_2) 
			{
				std::cerr << "Error: cannot creat output file " << outputFile+"_paired1 and " <<outputFile+"_paired2" << ".\nPlease double check.\nExiting..." << std::endl;
				exit (1);
			}
		}
		else
		{
			out = new std::ofstream {outputFile_1+"_paired1.fq"};
			out_2 = new std::ofstream {outputFile_2+"_paired2.fq"};
			if (!*out || !*out_2) 
			{
				std::cerr << "Error: cannot creat output file " << outputFile_1+"_paired1 and " <<outputFile_2+"_paired2" << ".\nPlease double check.\nExiting..." << std::endl;
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
		std::vector<std::string> read_vec ({ inputFile_1, inputFile_2 });
		std::map<int, std::vector< Fastq<TUPLETYPE> > > result;
		rcRatio = boost::lexical_cast <double> (reverseCompareMisRatio);
		geneRatio = boost::lexical_cast <double> (geneCompareMisRatio);
		adapterRatio = boost::lexical_cast <double> (adapterCompareMisRatio);


		FileReader < ParallelTypes::M_T, Fastq, TUPLETYPE, SOURCE_TYPE::IFSTREAM_TYPE > FileReader (read_vec, &result);    
		ParameterTrait <> i_parameter_trait (minLen, rcRatio, geneRatio, adapterRatio, 100000, nthreads);
		PairEndAdapterTrimmer <ParallelTypes::M_T, Fastq, TUPLETYPE, TrimTrait<std::string, LinearStrMatch <double>, double, double > > PEAT (i_parameter_trait);
		uint32_t count_reads(0);
		int flag_type (0);

		while (true)
		{
			bool eof = FileReader.Read (&result, 100000);
			PEAT.Trim (&result);
			count_reads += result[0].size();
			PEAT.Verbose( verboses, count_reads, flag_type );

			if (out != &std::cout)
			{
				for (auto& Q : result.begin()->second)
					(*out)<<Q;
				auto itr = result.begin();
				std::advance (itr, 1);
				for (auto& Q : itr->second)
					(*out_2)<<Q;
			}
			else
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
			static_cast<std::ofstream*>(out)-> close ();
			static_cast<std::ofstream*>(out_2)-> close ();
			delete out, out_2;
		}
		return 0;
	}
}
