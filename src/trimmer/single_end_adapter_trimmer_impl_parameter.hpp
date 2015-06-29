#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "../constant_def.hpp"

struct ParameterTraitSeat 
{
	int QS_index_, shift_value_, min_value_, max_value_;
	std::string adapter_seq_;
	double prior_, p_match_;
	int min_l_, match_score_, mismatch_score_;
	int min_read_length_;
	int threads_;
	int q_min_read_length_;
	float q_threshold_;
	std::vector<float> q_table_;
	
	ParameterTraitSeat (std::string adapterseq,
					std::string qtype,
					int threads = 1,
					float q_threshold = 30,
//					int q_min_read_length = 30,
					double prior=0.05, double pmatch=0.25, 
					int minlength=5, int matchscore=1, int mismatchscore=-1, int min_read_length=15 )
		: adapter_seq_ (adapterseq)
		, threads_ (threads)
		, prior_ (prior)
		, p_match_ (pmatch)
		, min_l_ (minlength)
		, match_score_ (matchscore)
		, mismatch_score_ (mismatchscore)
		, min_read_length_ (min_read_length)
//		, q_min_read_length_ (q_min_read_length)
		, q_threshold_ (q_threshold)
	{
		if (qtype=="PHRED")
			QS_index_ = QualityScoreType::PHRED;
		else if (qtype=="SANGER")
			QS_index_ = QualityScoreType::SANGER;
		else if (qtype=="SOLEXA")
			QS_index_ = QualityScoreType::SOLEXA;
		else if (qtype=="ILLUMINA")
			QS_index_ = QualityScoreType::ILLUMINA;

		switch (QS_index_)
		{
			case QualityScoreType::PHRED:
				shift_value_=0; min_value_=4; max_value_=60; break;
			case QualityScoreType::SANGER:
				shift_value_=33; min_value_=0; max_value_=93; break;
			case QualityScoreType::SOLEXA:
				shift_value_=64; min_value_=-5; max_value_=62; break;
			case QualityScoreType::ILLUMINA:
				shift_value_=64; min_value_=0; max_value_=62; break;
			default:
				std::cerr << "Please enter the correct quality-to-probability conversion formulas types." <<"\n";
				exit(1);
		}         
		
		float q_sum_temp(0);
		for (size_t index (0); index != 500; ++index) 
		{
			q_sum_temp += q_threshold_;
			q_table_.push_back(q_sum_temp);
		}
	}
};

template < 
		template <typename> class FORMAT, 
		typename TUPLETYPE
		>
class SingleEndAdapterTrimmer_impl
{
public:
	SingleEndAdapterTrimmer_impl ( ParameterTraitSeat traitin )
		: quality_score_trait_ (traitin)
	{}

	inline void QTrimImpl ( FORMAT<TUPLETYPE>& fq )
	{
		std::vector< float > sum_qscores;
		MapQualityToSumScore ( std::get<3>(fq.data), sum_qscores );
		size_t read_length ( std::get<1>(fq.data).size() );
		size_t cut_pos(0);
		while ( read_length > quality_score_trait_.min_read_length_ )
		{
			if ( sum_qscores[read_length-1] < quality_score_trait_.q_table_[read_length-1] )			
			{
				++cut_pos;
				--read_length;
			}
			else
				break;
		}
		std::get<1>(fq.data).resize ( std::get<1>(fq.data).size() - cut_pos );  
		std::get<3>(fq.data).resize ( std::get<3>(fq.data).size() - cut_pos );  
	}

	inline void TrimImpl ( FORMAT<TUPLETYPE>& fq, std::vector<int>& trim_pos )
	{
		std::vector< float > qprobs; 
		MapQualityToProb ( std::get<3>(fq.data), qprobs );
		std::tuple <bool, bool, int> best_shift = FindBestMatch ( std::get<1>(fq.data),	qprobs );
		if ( std::get<0>(best_shift), std::get<1>(best_shift) && std::get<2>(best_shift) > quality_score_trait_.min_read_length_ )
		{
			std::get<1>( fq.data ).resize (std::get<2>(best_shift));
			std::get<3>( fq.data ).resize (std::get<2>(best_shift));
			trim_pos.push_back (std::get<2>(best_shift));
		}
		else
			trim_pos.push_back (std::get<1>( fq.data ).size());
	}

protected:
	ParameterTraitSeat quality_score_trait_;

	inline void MapQualityToSumScore ( std::string qual_seq, std::vector<float>& scores )
	{
		float q_sum(0.0);
		int length = qual_seq.size(), quality_word;
		for ( int index = 0; index < length; ++index )
		{
			quality_word = (char) qual_seq[index]-quality_score_trait_.shift_value_;
			if ( quality_word < quality_score_trait_.min_value_ || quality_word > quality_score_trait_.max_value_ )
			{
				std::cerr << "Base quality out of range for specifed quality type." << std::endl;
				exit(1); 
			}
			q_sum += float(quality_word);
			scores.push_back(q_sum);
		}
	}

	inline void MapQualityToProb ( std::string qual_seq, std::vector<float>& probs )
	{
		int length = qual_seq.size(), quality_word;
		for ( int index = 0; index < length; ++index )
		{
			quality_word = (char) qual_seq[index]-quality_score_trait_.shift_value_;
			if ( quality_word < quality_score_trait_.min_value_ || quality_word > quality_score_trait_.max_value_ )
			{
				std::cerr << "Base quality out of range for specifed quality type." << std::endl;
				exit(1); 
			}
			switch (quality_score_trait_.QS_index_)
			{
				case QualityScoreType::SOLEXA: 
					probs.push_back( 1 - 1/( 1 + powf ( 10, (quality_word/10.0) ) ) ); break;
				case QualityScoreType::ILLUMINA: case QualityScoreType::SANGER: case QualityScoreType::PHRED:
					probs.push_back( 1 - 1/( powf ( 10, (quality_word/10.0) ) ) ); break;
			}
		}
	}

	inline std::tuple<bool, bool, int> FindBestMatch ( std::string& seq, std::vector< float >& qprobs )
	{
		bool match_flag = true; 
		bool is_con;
		std::vector< int > best_vec;
		std::vector< float > best_qprobs;
		int best_length=0, best_shift=0, best_score=quality_score_trait_.mismatch_score_*seq.size();//INT_MIN;
		for ( auto shift = 0; shift < seq.size()-quality_score_trait_.min_l_; ++shift )
		{
			if ( quality_score_trait_.min_l_ >= quality_score_trait_.adapter_seq_.size() )
			{
				std::cerr <<  "Minimum match length (option -n) greater than or equal to length of adapter.\n";
				std::cerr <<  "Minimum match length : length of adapter "<<quality_score_trait_.min_l_<<" : "<<quality_score_trait_.min_l_<<'\n';
				exit(1);
			}
			int current_length = std::min ( quality_score_trait_.adapter_seq_.size(), seq.size()-shift );
			std::string shift_seq (seq.begin()+shift, seq.end());
			std::vector <int> curr_vec (0);
			int curr_score = GetMatchingScore ( shift_seq, quality_score_trait_.adapter_seq_, current_length, curr_vec );
			if ( curr_score > best_score )
			{
				best_score = curr_score, best_length = current_length, best_shift = shift, best_vec = curr_vec;
				best_qprobs.clear();
				std::move (qprobs.begin()+shift, qprobs.end(), std::back_inserter(best_qprobs));
				is_con = GetPosterior ( best_vec, best_qprobs, best_length );
				if ( is_con )
					break;
				else
					;
			}
			else
				;
		}
		if (!is_con)
			match_flag = false;
		return std::make_tuple (match_flag, is_con, best_shift); 
	}

	inline int GetMatchingScore ( std::string seq, std::string pattern, int n, std::vector<int>& matches )
	{
		int sum (0);
		for ( int i = 0; i < n; i++ )
		{
			if ( seq[i] == pattern[i] )
			{
				matches.push_back( quality_score_trait_.match_score_ );
				sum+=quality_score_trait_.match_score_;
			}
			else
			{
				matches.push_back( quality_score_trait_.mismatch_score_ );
				sum+=quality_score_trait_.mismatch_score_;
			}			
		}
		return sum;
	}
	
	inline bool GetPosterior ( const std::vector< int > matches, std::vector< float > p_quals, int n ) 
	{
		std::pair <double,double> ls = GetLikelihood ( matches, p_quals, n );
//		double p_denom = quality_score_trait_.prior_*ls_con_ + (1-quality_score_trait_.prior_)*ls_ran_;	//same result should holds with/without p_denom dividing operation
		double ps_con = (ls.first * quality_score_trait_.prior_);// /p_denom; 
		double ps_ran = (ls.second * (1-quality_score_trait_.prior_)); // /p_denom;
		return ps_con > ps_ran;
	}

	inline std::pair<double, double> GetLikelihood ( const std::vector< int > matches, std::vector< float > p_quals, int n )
	{
		double ls_ran = 1.0;	
		double ls_con = 1.0;
		for ( int i = 0; i < n; i++ )
		{
			if ( matches[i] == 1 )
			{
				ls_ran *= quality_score_trait_.p_match_;
				ls_con *= p_quals[i];
			}
			else 
			{
				ls_ran *= 1 - quality_score_trait_.p_match_;
				ls_con *= 1 - p_quals[i];
			}
		}
		return std::make_pair (ls_con, ls_ran);
	}
	
	inline void VerboseImpl ( bool flag, uint32_t& read_count, int& flag_type )
	{
		if ( flag == true )
		{
			if ( flag_type == 0 )
			{	
				std::cerr << "=========================================================\n";
				std::cerr << "=====================Program process=====================\n";
				std::cerr << "The number of reads that are already trimmed:\n";
				++flag_type;
			}
			if ( flag_type == 2 )
			{
				std::cerr << "\n==================The program is over.===================\n";
			}
			else if ( flag_type == 1 )
				std::cerr << read_count << "\r";
			else
				;	
		}
	}
	
	inline void SumImpl ( std::vector< FORMAT<TUPLETYPE> >& result, uint32_t& sum_read_lengths, uint32_t& sum_read_counts )
	{
		for ( auto& fq: result )
		{
			sum_read_lengths+= std::get<1>( fq.data ).size();
			++sum_read_counts;
		}
	}	

};
