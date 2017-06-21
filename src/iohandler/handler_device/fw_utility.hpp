#ifndef FW_UTILITY_HPP_
#define FW_UTILITY_HPP_

struct FWBuffer
	:std::stringstream
{
	FWBuffer()
		:std::stringstream(std::stringstream::in|std::stringstream::out|std::stringstream::binary)
	{}
	typedef int intType;
	intType buffer_size()
	{
		intType tmp_p (this->tellg());
		this->seekp(0, std::ios::end);
		intType buffer_size = this->tellp();
		this->seekp(tmp_p);
		return buffer_size;
	}
	intType active_size()
	{
		return (intType) this->tellp() - this->tellg();
	}
	void refresh()
	{
		std::cout << "buffer refresh" << std::endl;
		intType copy_size = active_size();
		char* tmp = new char[copy_size+1];
		this->read(tmp, copy_size);
		if(copy_size != this->gcount())
		{
			std::cerr << "Error! copy_size != this->gcount()" << std::endl;
		}
		this->clear();
		this->seekg(0);
		this->seekp(0);
		this->write(tmp, copy_size);
		delete tmp;
	}
	template<class inputType>
	static intType buffer_size(inputType &in)
	{
		intType tmp_p (in.tellg());
		in.seekp(0, std::ios::end);
		intType buffer_size = in.tellp();
		in.seekp(tmp_p);
		return buffer_size;
	}
	template<class inputType, class outputType>
	static intType stream_copy(inputType &in, outputType &out, intType buffer_size = 4096, bool clear = false)
	{
		intType copy_size = buffer_size;
		intType active_size = in.tellp() - in.tellg();
		if(active_size == 0)
		{
			//如果buffer size == tellp 則檔案已經結尾（並且要設定成不清除flag才會執行）
			if(!clear && FWBuffer::buffer_size(in) == in.tellp())
				in.setstate(std::ios_base::eofbit);
			return 0;
		}
			
		if(active_size < buffer_size)
		{
			copy_size = active_size;
		}
		char* tmp = new char[copy_size+1];
		in.read(tmp, copy_size);
		out.write(tmp, copy_size);
		delete tmp;
		
		std::cout << "gcount " << in.gcount() << std::endl;
		if(clear)
		{
			in.clear();
		}
		return copy_size;
	}
};

#endif