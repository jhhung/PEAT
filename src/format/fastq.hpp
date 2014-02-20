/**
 * @file fastq.hpp
 * @brief Provide definition for Fastq Format.
 * @author JHH corp.
 */
#ifndef FASTQ_HPP_
#define FASTQ_HPP_
#include <string>
#include <tuple>
#include "../constant_def.hpp"
#include "../tuple_utility.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "is_tuple_type.hpp"
/**
 * @struct Fastq
 * @brief This is a Fastq struct, including member elements:
 *   static type (enum format_type);
 *   data (TUPLETYPE);
 *   eof_flag (bool)
 * This struct can store the fastq format data, provide constructor for move and copy. We also overload the operator<< for print the fastq's data.\n
 */
/**@tparam TUPLETYPE defaulted as tuple< string, string  >, indicate type of member element: data \n
 * defaulted as tuple of: \n
 * 1.string: Line1 begins with a '@' character and is followed by a sequence identifier.\n 
 * 2.string: Line2 is the raw sequence letters\n
 * 3.string: Line3 begins with a '+' character and is optionally followed by the same sequence identifier. \n
 * 4.string: Line4 encodes the quality values for the sequence in Line 2.
 */
template<class TUPLETYPE = std::tuple<std::string, std::string, std::string, std::string> >
struct Fastq
{
    static_assert( IsTupleType<TUPLETYPE>::value == true, "ARGUMENT IS NOT A TUPLE");
	TUPLETYPE data;
	///@brief An enumerated static identifier, indicating Fasta<TUPLETYPE> format struct
	static const format_types type = format_types::FASTQ;
	///@brief A flag indicates whether the Fasta data got by fasta_reader's get_next_entry function reaches the end-of-file.
	bool eof_flag;  //indicating whether file_handle reaches EOF
	///@typedef TupleType as a alias for TUPLETYPE
	typedef TUPLETYPE TupleType;
    int group_index_;

	///@brief Default constructor
	Fastq () 
        : data( TUPLETYPE() ), eof_flag( false ) //default constructor
        , group_index_ (Ungroup_default)
	{}
	///@brief Copy constructor
	Fastq ( const Fastq& in ) 
        : data( in.data ), eof_flag (in.eof_flag)//( false )
        , group_index_ (in.group_index_)
	{} //Copy constructor
	///@brief Assignment operator
	Fastq& operator = ( const Fastq& in )//Assignment operator
	{
		data = in.data;
		eof_flag = in.eof_flag;
        group_index_ = in.group_index_;                                                                                                                              
		return *this;
	}
	///@brief construct a Fastq <TUPLETYPE> with data_in_move in TUPLETYPE format
	///@param data_in_move
	//Move constructor for specific TUPLETYPE
	Fastq ( TUPLETYPE && data_in_move )
	    : data( std::move ( data_in_move ) )
	    , eof_flag ( false )
        , group_index_ (Ungroup_default)                                                                                                                                           
	{
//		std::string result;
//		TupleUtility< std::tuple<std::string, std::string, std::string, std::string>, std::tuple_size< TupleType >::value >::PrintTuple(data, result);
//		data_length_ = result.size();
	}
	///@brief construct a Fastq <TUPLETYPE> with data_in in TUPLETYPE format
	///@param data_in in TUPLETYPE format
    Fastq ( TUPLETYPE & data_in ) 
        : data ( data_in ), eof_flag (false)
        , group_index_(Ungroup_default)
    {
//		std::string result;
//		TupleUtility< std::tuple<std::string, std::string, std::string, std::string>, std::tuple_size< TupleType >::value >::PrintTuple(data, result);
//		data_length_ = result.size();
	}
    ///@brief Move construct for Fasta
    ///@param data 
	Fastq ( Fastq && other ) 
        : data( std::move( other.data ) ), eof_flag (std::move(other.eof_flag)) //Move constructor
        , group_index_ (std::move(other.group_index_))
    {}
	///@brief Move assignment operator
	///@param other
	Fastq& operator = ( Fastq && other ) //Move assignment operator
	{
		data = other.data;
		eof_flag = other.eof_flag;
        group_index_ = other.group_index_;                                                                                                                           
		return *this;
	}
    ///@brief construct a Fastq <TUPLETYPE> with an EOFFLAG in bool format
    ///@param EofFlag in bool format
	Fastq ( bool EofFlag ) 
        : eof_flag (EofFlag) 
    {} //end of file flag for fastq_reader io_end()
    /**@brief overload operator<< 
     * @param out ostream object
	 * @param s Fastq format data
     * @return struct Fastq <TUPLETYPE>
	 */
	friend std::ostream& operator<< ( std::ostream& out, const Fastq& s )
	{
		std::string result;
		TupleUtility< std::tuple<std::string, std::string, std::string, std::string>, std::tuple_size< TupleType >::value >::PrintTuple(s.data, result);
//		out << "@" << result;
		out << result;
		return out;
	}
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        TupleUtility < TUPLETYPE, std::tuple_size<TUPLETYPE>::value >
            :: SerializeTuple (data, ar, version);
        ar & eof_flag & group_index_;
    }

	size_t get_data_length (void)
	{
		std::string result;
		TupleUtility< std::tuple<std::string, std::string, std::string, std::string>, std::tuple_size< TupleType >::value >::PrintTuple(data, result);
		return result.size();
	}
	inline std::string getRevQuality () const {
		return std::string {std::get<3>(data).rbegin(), std::get<3>(data).rend()};
    }
	inline std::string getName()
	{
		return std::get<0>(data).substr(1);
	}
	inline std::string getSeq()
	{
		return std::get<1>(data);
	}
	inline std::string getName2()
	{
		return std::get<2>(data);
	}
	inline std::string getQuality()
	{
		return std::get<3>(data);
	}
};
#endif
