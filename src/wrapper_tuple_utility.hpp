/// @file Wrapper_tuple_utility.hpp 
/// @brief Served as an adapter class, for extending TupleUtility::FillTuple's capability to achieve boost::lexical_cast on currently lexical_cast-incastable format, e.g. std::tuple <> and std::vector <> 
/// @author C-Salt Corp.
#ifndef WRAPPER_TUPLE_UTILITY
#define WRAPPER_TUPLE_UTILITY
#include "../include/boost_include.hpp"
#include "../include/stdlib_include.hpp"
#include "tuple_utility.hpp"
#include "constant_def.hpp"

/// @class Wrapper_Impl template class
/// @brief  has a variatic parameter list, wherein only an int / enum, i.e. non-type, and a type TUPLETYPE are identified\n include member elements:\n data_content (TUPLETYPE) \n
/// First specialized Wrapper <WrapperType::TUPLE_WRAPPER, TUPLETYPE> template class, with its non-type parameter WrapperType Specialized to the value of WrapperType::TUPLE_WRAPPER, indicating that data_content has a structure with std::tuple \n Specially served to have the boost::lexical_cast adapted for casting data having std::tuple structure \n
/// Second specialized Wrapper <WrapperType::VECTOR_WRAPPER, VECTORTYPE> template class, with its non-type parameter WrapperType Specialized to the value of WrapperType::VECTOR_WRAPPER, indicating that data_content has a structure with std::vector \n Specially served to have the boost::lexical_cast adapted for casting data having std::vector structure
/// @tparam none-type parameter of int/enum of WrapperType, indicating what kind of format the data_content has.
template <int, typename...>
class Wrapper_Impl
{};

/// @brief specialized form of the Wrapper_Impl class, with the WrapperType specialized to be TUPLE_WRAPPER, indicating that the member element data_content has the type of std::tuple <...>
template <class TUPLETYPE>
class Wrapper_Impl<WrapperType::TUPLE_WRAPPER, TUPLETYPE> 
{
public:
	///@typedef TupleType as a alias for TUPLETYPE
	typedef TUPLETYPE MyType;
	///@brief the element actually holding data content, having the type of std::tuple<...> in the present template class specialization
	MyType data_content;
	
	///@brief default constructor
	Wrapper_Impl () 
		: data_content ( MyType() )
	{}//default constructor

	///@brief constructor from input data having the type of TUPLETYPE
	Wrapper_Impl (MyType& In)
		: data_content(In)
	{}

	///@brief copy constructor
	Wrapper_Impl (const Wrapper_Impl& Wp)	//copy constructor
		: data_content(Wp.data_content)
	{}

	///@brief assignment operator
	Wrapper_Impl& operator = (const Wrapper_Impl& Wp)	//assignment
	{
		data_content = Wp.data_content;
		return *this;
	}

	///@brief move constructor
	Wrapper_Impl (Wrapper_Impl&& Wp)	//move constructor
		: data_content(std::move(Wp.data_content))
	{}

	///@brief move assignment operator
	Wrapper_Impl& operator = (Wrapper_Impl&& Wp)	//move assignment
	{
		data_content = std::move(Wp.data_content);
		return *this;
	}

/// @memberof Wrapper_Impl <WrapperType::TUPLE_WRAPPER, TUPLETYPE>
/// @brief Served as a relatively straightforward interface for getting elements of the data_content, in TUPLETYPE format
/// @param N non-type int, served as an index for elements within the data_content
/// @return std::tuple_element, i.e. the n-th element of the data_content
	template <int N>
	typename std::tuple_element<N, TUPLETYPE>::type get(void)
	{
		return std::get<N>(data_content);
	}

/// @memberof Wrapper_Impl <WrapperType::TUPLE_WRAPPER, TUPLETYPE>
/// @brief With overloaded operator <<, for providing an efficient interface for printing the data_content, in TUPLETYPE format\n
/// implemented with function TupleUtility::PrintTuple, defined in TupleUtility.hpp
/// @param std::ostream& object o
/// @param a Wrapper_Impl<WrapperType::TUPLE_WRAPPER, TUPLETYPE> object
	inline friend std::ostream& operator << (std::ostream& out, const Wrapper_Impl& Wp)	
	{
		TupleUtility<MyType, std::tuple_size<MyType>::value>:: PrintTuple(out, Wp.data_content);
		return out;
	}

/// @memberof Wrapper_Impl <WrapperType::TUPLE_WRAPPER, TUPLETYPE>
/// @brief With overloaded operators >>, for instructing boost::lexical_cast how to manage cast operation on Wrapper_Impl class\n
/// @param std::istream& object In
/// @param a Wrapper_Impl<WrapperType::TUPLE_WRAPPER, TUPLETYPE> object
	inline friend std::istream& operator >> (std::istream& In, Wrapper_Impl& Wp)	// overloading operation >> for enabling lexical_cast on UDT
	{																		// current Wrapper_Impl<TUPLE_WRAPPER,TUPLETYPE> only support the TUPLETYPE in the type of
																			// Wrapper_Impl < 0, tuple < TWO ELEMENTS > >
																			// for handling lexical_cast opertion in get_next_entry function of wig format
		In.ignore (1024, '*');		//find the delimiting character of '*', and discard the character of '*' with data coming before it
		In >> std::get<0> (Wp.data_content);	//obtain string comes after '*' and before '#', and output it
		In.ignore (1024, '#');		//find the delimiting character of '#', and discard the character of '#' with data coming before it
		In >> std::get<1> (Wp.data_content);	//obtain string comes after '#' and output it
		In.setstate (std::ios::eofbit);		//have the eofbit of the ios object, i.e. istream here, to indicate the end of lexical_cast operation
		return In;
	}

    friend class boost::serialization::access;
    template<class Archive>
/// @memberof Wrapper_Impl <WrapperType::TUPLE_WRAPPER, TUPLETYPE>
/// @brief provide serialize function, so as to indicate the rule to serialize current Wrapper_Impl class.  Serialization is the key to achieve MPI operation
    void serialize(Archive &ar, unsigned int)
    {
        ar & std::get<0>(data_content) & std::get<1>(data_content);
    }
};

/// @brief specialized form of the Wrapper_Impl class, with the WrapperType specialized to be VECTOR_WRAPPER, indicating that the member element data_content has the type of std::vector <...>
template <class VECTORTYPE>
class Wrapper_Impl<WrapperType::VECTOR_WRAPPER, VECTORTYPE> 
{
public:
    ///@typedef TupleType as a alias for TUPLETYPE
	typedef VECTORTYPE MyType;
    ///@brief the element actually holding data content, having the type of std::tuple<...> in the present template class specialization
	MyType data_content;

    ///@brief default constructor
	Wrapper_Impl ()
		: data_content( MyType() ) 
	{}	//empty constructor 

    ///@brief constructor from input data having the type of TUPLETYPE
	Wrapper_Impl (MyType& In)
		: data_content (In)
	{}

    ///@brief copy constructor
	Wrapper_Impl (const Wrapper_Impl & Wp)	//copy constructor
		: data_content(Wp.data_content) 
	{}

    ///@brief assignment operator
	Wrapper_Impl& operator = (const Wrapper_Impl& Wp)	//assignment
	{
		data_content = Wp.data_content;
		return *this;
	}

    ///@brief move constructor
	Wrapper_Impl (Wrapper_Impl && Wp)	//move constructor
		: data_content (std::move(Wp.data_content)) 
	{}

    ///@brief move assignment operator
	Wrapper_Impl& operator = (Wrapper_Impl&& Wp)	//move assignment
	{
		data_content = std::move (Wp.data_content);
		return *this;
	}

/// @memberof Wrapper_Impl <WrapperType::VECTOR_WRAPPER, VECTORTYPE>
/// @brief Served as a relatively straightforward interface for getting elements of the data_content, in VECTORTYPE, e.g. std::vector, format
/// @param N non-type int, served as an index for elements within the data_content
/// @return VECTORTYPE::value_type, i.e. the n-th element of the data_content
	template <int N>
	typename VECTORTYPE::value_type get(void)
	{
		return data_content[N];
	}

/// @memberof Wrapper_Impl <WrapperType::VECTOR_WRAPPER, VECTORTYPE>
/// @brief With overloaded operator <<, for providing an efficient interface for printing the data_content, in VECTORTYPE format\n
/// implemented with function TupleUtility::PrintTuple, defined in TupleUtility.hpp
/// @param std::ostream& object o
/// @param a Wrapper_Impl<WrapperType::VECTOR_WRAPPER, VECTORTYPE> object
	inline friend std::ostream& operator << (std::ostream& out, const Wrapper_Impl & Wp)
	{
		std::for_each(
						Wp.data_content.begin(), 
						Wp.data_content.end(), 
						[&](const typename VECTORTYPE::value_type data_element)
							{ out << data_element ; } 
					);
        return out;
    }

/// @memberof Wrapper_Impl <WrapperType::VECTOR_WRAPPER, VECTORTYPE>
/// @brief With overloaded operators >>, for instructing boost::lexical_cast how to manage cast operation on Wrapper_Impl class\n
/// @param std::istream& object In
/// @param a Wrapper_Impl<WrapperType::VECTOR_WRAPPER, VECTORTYPE> object
	inline friend std::istream& operator >> (std::istream& In, Wrapper_Impl & Wp)	//overloading operator >> for enabling lexical_cast on UDT
	{
		int count = 0;
		while (1)
		{
			std::istream temp(In.rdbuf());	
				//construct another istream object temp according to rdbuf of input istream object In.  We can't directly use In 
				//in the following operator>> operation, since the eof flag of the istream involved therein	will be set
			In.peek();
				//check In's flag, and break the while loop if In.eof() is set
			if (In.eof())
				break;
			Wp.data_content.push_back(typename VECTORTYPE::value_type());	
				//push back an empty object before the operation of dumping istream content into Wp.data_content requred by next line
				//in order to ensure the dumping operation, required by lexical_cast, operate properly
			temp >> Wp.data_content[count];
			count++;
		}
		In.setstate (std::ios::eofbit);	
				//set std::ios::eofbit to be true after lexical_cast operation, to indicate the end of lexical_cast operation
		return In;
	}

    friend class boost::serialization::access;
    template<class Archive>
/// @memberof Wrapper_Impl <WrapperType::VECTOR_WRAPPER, TUPLETYPE>
/// @brief provide serialize function, so as to indicate the rule to serialize current Wrapper_Impl class.  Serialization is the key to achieve MPI operation
    void serialize(Archive &ar, unsigned int)
    {
        ar & data_content;
    }
};

/// @class Wrapper_Policy template class
/// @brief served as a simple type verifier to provide the corresponding integer values of 1 and 2 to respectively indicate that the ellipsis template parameter T... corresponds to a std::tuple<...> or a std::vector<...> format.
/// @tparam an ellipsis type parameter T..., selectively corresponding to std::tuple<...> or std::vector<...> format
template < typename... T >
struct Wrapper_Policy
{
	///@brief the value is, by default, equal to 0
	static const int value = 0; 
};

/// @brief specialized form of the Wrapper_Policy, with the ellipsis template parameter T... specialized to be std::tuple<...> format
template < typename... T >
struct Wrapper_Policy < std::tuple <T...> >
{
	///@brief the value is assigned to be the value of 1 as the template parameter T... is specialized to be std::tuple <...> format 
	static const int value = 1;
};

/// @brief specialized form of the Wrapper_Policy, with the ellipsis template parameter T... specialized to be std::vector<...> format
template < typename... T >
struct Wrapper_Policy < std::vector <T...> >
{
	///@brief the value is assigned to be the value of 2 as the template parameter T... is specialized to be std::vector <...> format 
    static const int value = 2;
};

/// @class Wrapper template class
/// @brief publicly derived from the aforementioned Wrapper_Impl, to provide a union interface for Wrapper class with data_content in either std::tuple<...> or std::vector<...> formats.  In the Wrapper template class, the static const Wrapper_Policy<T>::value is employed to provide the value, selectively corresponding to one the enumerated value of 1, i.e. WrapperType::TUPLE_WRAPPER, and 2, i.e. WrapperType::VECTORTYPE, as a navigator for automatic instantiate the corresponding Wrapper_Impl class specialization forms.
/// @tparam a type parameter T, has the same type as the template parameter TUPLETYPE / VECTORTYPE 
template < class T >
class Wrapper
	: public Wrapper_Impl < Wrapper_Policy <T> :: value, T >
{
public:
	///@brief default constructor
	Wrapper ()
		: Wrapper_Impl <Wrapper_Policy<T>::value, T> ()
	{}

	///@brief constructor from input data having the type of TUPLETYPE
	Wrapper (T a)
		: Wrapper_Impl <Wrapper_Policy<T>::value, T > (a)
	{}
};

#endif
