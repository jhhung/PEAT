//#include <fstream>
//#include <sstream>
//#include <iterator>
#include "iohandler/ihandler/ihandler.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "iohandler/ihandler/IhandlerFactory.h"

//template < template < typename> class FORMAT, typename TUPLETYPE > 
//IhandlerFactory<FORMAT, TUPLETYPE>::IhandlerFactory<FORMAT, TUPLETYPE>()
//{}

template < 
	template < 
		typename
	> class FORMAT, 
	typename TUPLETYPE 
> std::tuple<std::function<bool(std::map< int, std::vector< FORMAT <TUPLETYPE> > >*, size_t)>, std::function<void(void)>>
IhandlerFactory<FORMAT, TUPLETYPE>::get_ihandler_read ( std::vector<std::string> read_vec, std::map <int, std::vector< FORMAT<TUPLETYPE> > >& content)
{
	std::vector<std::string> str_temp;
	boost::split ( str_temp, read_vec[0], boost::is_any_of (".") );
	if ( *(str_temp.end()-1) == "gz" )
	{
		auto ptr = new IHandler < ParallelTypes::NORMAL, FORMAT, TUPLETYPE, IoHandlerGzipIfstream > (read_vec, &content);
		std::function<bool(std::map< int, std::vector< FORMAT <TUPLETYPE> > >*, size_t)> read ( 
			[ptr](std::map< int, std::vector< FORMAT <TUPLETYPE> > >* result, size_t Num = 1)->bool 
			{ 
				return ptr->Read(result, Num);
			}
		);
		std::function<void(void)> deletor(
			[ptr]()
			{
				delete ptr;
			}
		);
		ptr = NULL;
		return std::make_tuple(read, deletor); 
	}	
	else
	{
		auto ptr = new IHandler < ParallelTypes::NORMAL, FORMAT, TUPLETYPE, IoHandlerIfstream > (read_vec, &content);
		std::function<bool(std::map< int, std::vector< FORMAT <TUPLETYPE> > >*, size_t)> read (
			[ptr](std::map< int, std::vector< FORMAT <TUPLETYPE> > >* result, size_t Num = 1)->bool 
			{ 
				return ptr->Read(result, Num);
			}
		);
		std::function<void(void)> deletor(
			[ptr]()
			{
					delete ptr;
			}
		);
		ptr = NULL;
		return std::make_tuple(read, deletor);	
	}	
}
template class IhandlerFactory<Fastq, std::tuple<std::string, std::string, std::string, std::string>> ;
