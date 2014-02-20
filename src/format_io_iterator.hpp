/// @file format_io_iterator.hpp
/// @brief Provide an iterator-based interface for the get_next_entry function of FileReader.hpp
/// @author C-Salt Corp.
#ifndef FORMAT_IO_ITERATOR_HPP_
#define FORMAT_IO_ITERATOR_HPP_
#include <boost/iterator/iterator_facade.hpp>
#include <memory>
#include <iosfwd>
/// @class FormatIoIterator 
/// @brief Include member elements:\n file_handle (std::ifstream*);\n data_entry (std::shared_ptr<T>);\n
/// boost::iterator_facade is employed for implementing FormatIoIterator
/// @tparam T indicating the output data format, e.g. Wig <TUPLETYPE>, pointed to by the FormatIoIterator
/// @tparam FORMAT indicating the FileReader_impl format, e.g. FileReader_impl< format_types::WIG, TUPLETYPE>, employing the FormatIoIterator
template <typename T, typename FORMAT>
class FormatIoIterator : 
	public boost::iterator_facade<
	FormatIoIterator<T, FORMAT>,
	T,
	boost::forward_traversal_tag
	>
{
public:
	explicit FormatIoIterator(std::shared_ptr<T> ptr)
	    : data_entry(ptr)
	{}
	FormatIoIterator()
	{}
	FormatIoIterator (const FormatIoIterator &other)
	    : file_handle(other.file_handle), data_entry(other.data_entry)
	{}
/// @memberof FormatIoIterator
/// @brief a constructor based explicitly on a std::ifstream pointer fin, wherein the data_entry is reset as a pointer pointing to a return value of FORMAT::get_next_entry, i.e. output data, such as Wig <TUPLETYPE>.    
/// @param fin in std::ifstream pointer type
	explicit FormatIoIterator(std::ifstream* fin)
	    : file_handle(fin)
	{
	    data_entry.reset( new T(FORMAT::get_next_entry(*file_handle) ) );
	}
public://private:
/// @memberof FormatIoIterator
/// brief file_handle, in std::ifstream pointer format
	std::ifstream* file_handle;
/// @memberof FormatIoIterator
/// brief data_entry, in shared_ptr pointer format pointing to outputdata
	std::shared_ptr<T> data_entry;
/// @memberof FormatIoIterator
/// @brief judge whether the two output data objects (e.g. in Wig<TUPLETYPE> format) , which are respectively pointed to by the two FormatIoIterator objects, other and aforementioned FormatIoIterator, have a same eof_flag value
/// @param object other in FormatIoIterator<T, FORMAT>& type
/// @return bool indicating objects other and aforementioned FormatIoIterator have a same eof_flag value
	friend class boost::iterator_core_access;
	bool equal(FormatIoIterator<T, FORMAT> const& other) const
	{
		// equal is only used to test eof 
	    return this->data_entry->eof_flag == other.data_entry->eof_flag;
	}
/// @memberof FormatIoIterator
/// @brief define dereference operation for FormatIoIterator class
/// @return T&, i.e. the output data format 
	T& dereference() const
	{
	    return *data_entry;
	}
/// @memberof FormatIoIterator
/// @brief define increment operation for FormatIoIterator class
	void increment()
	{
		data_entry.reset( new T(FORMAT::get_next_entry(*file_handle) ) );
	}
};


#endif
