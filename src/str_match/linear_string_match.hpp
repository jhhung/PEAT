#include <algorithm>
#include <string>
#include <iostream>
#include "boost/range.hpp"
#include <vector>
template <typename MISMATCH_INDICATOR, typename STRING = boost::iterator_range <std::string::iterator> >//std::string>
class LinearStrMatch
{};

template <> 
class LinearStrMatch < size_t, std::string >
{
public:
	typedef size_t mismatch_indicator_type;
	std::string pattern;
	size_t pattern_length, txt_length;
	std::string alphabet;
	size_t mismatch_count;

public:
    LinearStrMatch (std::string& str_pattern, size_t mmc_in)
        : pattern (str_pattern)
        , pattern_length (pattern.size())
        , alphabet ("ACGNT")
		, mismatch_count (mmc_in)
    {}

	bool find (std::string& txt, std::vector<size_t>& found_vec)
	{   
		txt_length = txt.size();
		if (txt_length<pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i<=txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && (pattern[j]==txt[i+j] || mmc!=0) )
			{
				if (pattern[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				found_vec.push_back (i+pattern_length);//(i+jj-pattern_length);
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	bool find (std::string& txt)
	{   
		txt_length = txt.size();
		if (txt_length<pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i<=txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && (pattern[j]==txt[i+j] || mmc!=0) )
			{
				if (pattern[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	void Print (void)
	{
		std::cerr<<"alphabet "<<std::endl;
		std::cerr<<alphabet<<std::endl;
	}
};

template <>
class LinearStrMatch < double, std::string >//StrMatchScheme::DOUBLE_TYPE, MISMATCH_INDICATOR >
{
public:
	typedef double mismatch_indicator_type;
	std::string pattern;
	size_t pattern_length, txt_length;
	std::string alphabet;
	double mismatch_ratio;
	size_t mismatch_count;

public:
    LinearStrMatch (std::string& str_pattern, double mmc_in)
        : pattern (str_pattern)
        , pattern_length (pattern.size())
        , alphabet ("ACGNT")
		, mismatch_ratio (mmc_in)
		, mismatch_count (mismatch_ratio* pattern_length)
    {}

	bool find (std::string& txt, std::vector<size_t>& found_vec)
	{   
		txt_length = txt.size();
		if (txt_length<pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i<=txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && (pattern[j]==txt[i+j] || mmc !=0) )
			{
				if (pattern[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				found_vec.push_back (i+pattern_length);//(i+jj-pattern_length);
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	bool find (std::string& txt)
	{   
		txt_length = txt.size();
		if (txt_length<pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i<=txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && (pattern[j]==txt[i+j] || mmc!=0) )
			{
				if (pattern[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	void Print (void)
	{
		std::cerr<<"alphabet "<<std::endl;
		std::cerr<<alphabet<<std::endl;
	}
};

template <>
class LinearStrMatch < size_t >//, boost::iterator_range <std::string::iterator> >
{
public:
	typedef size_t mismatch_indicator_type;
	boost::iterator_range <std::string::iterator> pattern;
	size_t pattern_length, txt_length;
	std::string alphabet;
	size_t mismatch_count;
public:
    LinearStrMatch ( boost::iterator_range <std::string::iterator> str_pattern, size_t mmc_in )
        : pattern (str_pattern)
        , pattern_length (pattern.size())//(pattern.size())
        , alphabet ("ACGNT")
		, mismatch_count (mmc_in)
    {}

	bool find (boost::iterator_range <std::string::iterator> txt, std::vector<size_t>& found_vec)
	{   
		txt_length = txt.size();
		if (txt_length < pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i <= txt_length - pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && ( pattern[j] == txt[i+j] || mmc!=0) )
			{
				if ( pattern[j] != txt[i+j] )
					--mmc;
				--j;
			}
			if (j < 0)
			{
				found_vec.push_back (i+pattern_length);//(i+jj-pattern_length);
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	bool find (boost::iterator_range <std::string::iterator> txt)
	{   
		txt_length = txt.size();
 		if ( txt_length < pattern_length )
			return false;
		
		bool return_value = false;
		size_t i = 0,  jj = pattern_length-1;
		while ( i <= txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j = jj;
			while ( j >= 0 && ( pattern[j] == txt[i+j] || mmc != 0) )
			{
				if ( pattern[j] != txt[i+j] )
					--mmc;
				--j;
			}
			if (j < 0)
			{
				return_value = true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	void Print (void)
	{
		std::cerr<<"alphabet "<<std::endl;
		std::cerr<<alphabet<<std::endl;
	}
};

template <>
class LinearStrMatch < double >//, boost::iterator_range <std::string::iterator> >
{
public:
	typedef double mismatch_indicator_type;
	boost::iterator_range <std::string::iterator> pattern;
	size_t pattern_length, txt_length;
	std::string alphabet;
	double mismatch_ratio;
	size_t mismatch_count;

public:
    LinearStrMatch (boost::iterator_range <std::string::iterator> str_pattern, double mmc_in)
        : pattern (str_pattern)
        , pattern_length (pattern.size())
        , alphabet ("ACGNT")
		, mismatch_ratio (mmc_in)
		, mismatch_count (mismatch_ratio* pattern_length)
    {}

	bool find (boost::iterator_range <std::string::iterator> txt, std::vector<size_t>& found_vec)
	{   
		txt_length = txt.size();
		if (txt_length<pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i<=txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && (pattern[j]==txt[i+j] || mmc !=0) )
			{
				if (pattern[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				found_vec.push_back (i+pattern_length);//(i+jj-pattern_length);
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	bool find (boost::iterator_range <std::string::iterator> txt)
	{   
		txt_length = txt.size();
		if (txt_length<pattern_length)
			return false;
		
		bool return_value=false;
		size_t i=0,  jj=pattern_length-1;
		while ( i<=txt_length-pattern_length )
		{
			size_t mmc = mismatch_count;
			int j=jj;
			while ( j>=0 && (pattern[j]==txt[i+j] || mmc!=0) )
			{
				if (pattern[j]!=txt[i+j])
					--mmc;
				--j;
			}
			if (j < 0)
			{
				return_value=true;
				++i;
			}
			else
				++i;
		}
		return return_value;
	}

	void Print (void)
	{
		std::cerr<<"alphabet "<<std::endl;
		std::cerr<<alphabet<<std::endl;
	}
};

