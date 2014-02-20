//#include "../include/includes.hpp"
#include <boost/filesystem.hpp>
#include "trimmer/peat.hpp"
#include "trimmer/seat.hpp"
//using namespace std;
int main (int argc, char** argv) 
{
	std::string usage = R"(

*********************************************************************************
+----+
|PEAT|
+----+
  A integrated software that can do either paired-end and single-end adapter trimming operation.  

Usage:

 1. paired-end adapter trimming
>	PEAT paired --help
  
 2. single-end adapter trimming
>	PEAT single --help
*********************************************************************************

)";
	if (argc < 2) {
		std::cerr << usage << std::endl;
		exit (1);
	}
	if (strcmp (argv[1], "paired") == 0) 
		paired_end::main (argc-1, argv+1);
	else if (strcmp (argv[1], "single") == 0) 
		single_end::main (argc-1, argv+1);
	else 
	{
		std::cerr << "Error: unrecognized option " << argv[1] << std::endl;
		std::cerr << usage << std::endl;
		exit (1);
	}
}
