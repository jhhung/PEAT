#ifndef TOPHAT_HPP_
#define TOPHAT_HPP_

/* I have rearranged how functions of tailer are stored in header files
 * buildBWT is no longer buried in namespace bowhan anymore, so redefining buildBWT here cause compiler error
 * I hope that you don't mind I commented out your code here
 * please locate buildBWT in header file src/tailer.hpp
 * But since you already include abit_include.hpp in your tophat_test.cpp, I  guess that it doesn't matter
 * also note that there are buildBWT and buildBWT2 now
 * the first built BWT for + and - separately
 * the later build BWT for concatenated + and - sequence
 */

//void buildBWT (const std::string& fileName, const std::string& prefixName)
//{
//	/* read input fasta file */
//	std::ifstream in {fileName};
//	/* string to store the sense/antisense index file; currently we store them simultaneously, if RAM is not enough, we do them separately*/
//	std::string seq {};
//	/* running accumulator recording the length of each chr */
//	uint64_t tempLen {0}, accumulatedLength {0};
//	/* for concatenated seq */
//	std::map <uint64_t, uint64_t> NPosLen { };
//
//	/* file to store which regions has which chr*/
//	std::ofstream chrStartPos {prefixName + ".chrStart"};
//	/* file to store the length of each chr */
//	std::ofstream chrLen {prefixName + ".chrLen"};
//	/* read in each fasta and make two string */
//	while (in.good ()) {
//		Fasta<std::vector> fa {in};
//		/* store start position of each chr */
//		chrStartPos << fa.getName () << '\t' << accumulatedLength << '\n';
//		/* get chr length */
//		tempLen = fa.getLengthNoN ();
//		/* store chr length */
//		chrLen << fa.getName () << '\t' << tempLen << '\n';
//		/* update accumulated length */
//		accumulatedLength += tempLen;
//		/* update NPosLen */
//		fa.updateNpos (NPosLen);
//		seq += fa.getSeqNoN ();
//	}
//
//	chrStartPos.close ();
//	chrLen.close ();
//	/* get RC */
//	std::string seqRC {seq.rbegin(), seq.rend()};
//	for (auto iter = seqRC.begin(); iter!= seqRC.end(); ++ iter) {
//		switch (*iter) {
//			case 'A':
//				*iter = 'T'; break;
//			case 'T':
//				*iter = 'A'; break;
//			case 'G':
//				*iter = 'C'; break;
//			case 'C':
//				*iter = 'G'; break;
//		}
//	}
//	/* append $ */
//	seq += '$';
//	seqRC += '$';
//	/* writing NPosLen to file */
//	{
//		boost::iostreams::filtering_ostream fos;
//		fos.push (boost::iostreams::zlib_compressor());
//		fos.push (boost::iostreams::file_sink (prefixName + ".NposLen.z"));
//		boost::archive::binary_oarchive oa (fos);
//		oa << NPosLen;
//	}
//	//
//	{
//		ABSequence<std::string> x ( seq );
//		ABWT<ABSequence<std::string>> y (x, 512, 64, prefixName);
//	}
//	{
//		ABSequence<std::string> x ( seqRC );
//		ABWT<ABSequence<std::string>> y (x, 512, 64, prefixName + "RC");
//	}
//}
class QQ
{
public:
	QQ()
	{
		std::cerr<<"you"<<std::endl;
	}	

};

#endif 
