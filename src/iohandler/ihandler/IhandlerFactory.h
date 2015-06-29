#ifndef FILE_READER_SELECTOR_HPP_
#define FILE_READER_SELECTOR_HPP_
#include <vector>
#include <tuple>
#include <functional>
#include <map>

template < template < typename> class FORMAT, typename TUPLETYPE >
class IhandlerFactory
{
public:
	static std::tuple<std::function<bool(std::map< int, std::vector< FORMAT <TUPLETYPE> > >*, size_t)>, std::function<void(void)>>
	get_ihandler_read (std::vector<std::string> read_vec, std::map <int, std::vector< FORMAT<TUPLETYPE> > >& content);
};


#endif
