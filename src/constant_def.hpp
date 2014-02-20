/// @file constant_def.hpp
/// @brief provide enum format_types, CompressType, and WrapperType as navigating indexes for respectively achieving tag techniques on specialized FileReader, compression, and Wrapper template classes
/// @author C-Salt Corp.
#ifndef CONSTANT_DEF_HPP_
#define CONSTANT_DEF_HPP_

#include <string>

std::string ALPHABET_$ACGNT("$ACGNT");
std::string ALPHABET_3CHAR(" \"#$%&\'()*+,-./012345;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz");
#define gAlphabet &ALPHABET_$ACGNT

#define INTTYPE uint64_t
#define Ungroup_default -5566                                                                                                                                        

///@enum format_types
///@brief format_types, assigning enumerated int, e.g. 0, 1, 2, 3, ... for indicating different formats, e.g. FastA, FastQ, Bed, Wig, ... .
enum format_types
{
    FASTA,
    FASTQ,
    BED,
	WIG,
	SAM
};
///@enum CompressTypes
///@brief CompressType, assigning enumerated int, e.g. 0, 1, ... for indicating different compression schemes, e.g. Plain, TWO_BITS, ... .
enum CompressType
{
    Plain,
    N_BITS, ///two bits, A->00 C->01 G->10 T->11, the order of binary is the same as the lexigraphical order
	N_BITS_MASK,
	LOWERCASE
};
///@enum WrapperType
///@brief WrapperType, assigning enumerated int, e.g. 0, 1, ... for indicating different wrapper formats, e.g. TUPLE_WRAPPER, VECTOR_WRAPPER, ... .
enum WrapperType
{
	TUPLE_WRAPPER = 1,
	VECTOR_WRAPPER =2,
};
///@enum ParallelTypes
///@brief ParallelType, assigning enumerated int for indicating different file_reader_intf policies, e.g. normal, multi-thread, ... .
enum ParallelTypes
{
	NORMAL,
	M_T,
	M_P_I
};

enum TwoBit_types
{   
    Normal,
    Mask
};

enum CompressFormat
{
    GZ,
    PLAIN,
};

enum SOURCE_TYPE
{
    IFSTREAM_TYPE,
    CURL_TYPE_GZ,
    CURL_TYPE_PLAIN   //not tested yet
};

enum Genome_strand_types
{
	Forward_strand,
	Reverse_strand,
	Dual_strand
};

//Aligner type
enum Aligner_types
{
	BWT_Aligner,
	BLAST_Aligner
};

//search_type

enum Searcher_types
{
	Default,
	Tailer,
	Exact_match
};

//used in single-end-adapter-trimmer_impl
enum QualityScoreType                                                                                                                                                 
{
    PHRED,
    SANGER,
    SOLEXA,
    ILLUMINA
};  

//SAM Flag int value 
enum SAM_FLAG 
{
	MAPPED = 0,
	PAIRED_END = 1,
	EACH_END_ALIGNED = 2,
	UNMAPPED = 4,
	NEXT_UNMAPPED = 8,
	REVERSE_COMPLEMENTED = 16,
	NEXT_REVERSE_COMPLEMENTED = 32,
	FIRST_SEG = 64,
//FUCKER = 89,
	SECOND_SEG = 128,
	SECONDARY_ALIGNMENT = 256,
	NOT_PASSING_QUALITY = 512,
	PCR_DUP = 1024,
	FLAG_SIZE = 11
};

//search_type for old
/*
enum BWT_SEARCHING_TYPE
{
	Default,
	Tailer,
	Exact_match
};
*/

// impl for barcode version
enum BarcodeHandleScheme
{
    Five_Prime,
    Three_Prime
};

enum AnnoIgnoreStrand
{
	IGNORE,
	NO_IGNORE
};
enum AnnoType
{
	INTERSET
};

enum AnalyzerTypes
{
	LengthDistribution,
	Heterogeneity,
	ToBam,
	ToBwg
};

enum HeterogeneityClusterCoding                                                                                              
{
    FivePrimeHead,
    FivePrimeTail,
    ThreePrimeHead,
    ThreePrimeTail
};

#endif
