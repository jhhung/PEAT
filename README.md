PEAT
=========
PEAT, a fast and highly accurate adapter-trimming algorithm, designed specifically for paired-end sequencing. PEAT requires no a priori adapter sequence and significantly outperforms other widely used adapter trimmers in both simulated and real life paired-end sequencing libraries. Moreover, in order to process single-end sequencing data, we also provide single-end trimmer in PEAT.

PEAT is released by JHHLAB with additional restriction so that any copyright infringement is not permitted.

##INSTALL

*Only 64 bits systems are able to compile and run PEAT. 

#### Install the dependencies

- 1.1 Relative recent C++ compiler that support most features of C++11. We recommend [GCC](http://gcc.gnu.org/).
- 1.2 [Boost](http://www.boost.org/users/download/)
- 1.3 [CMake](http://www.cmake.org/)

#### Get the latest version of the software

```
git clone git@github.com:jhhung/PEAT.git
```

#### Enter the folder PEAT and:

- Set enviromental variable "BOOST_ROOT" to the directory of boost if CMake cannot find boost automatically;
- Set enviromental variable "CC" and "CXX" to the gcc/g++ compiler you want to use.	

```
cmake .
```
   
	
#### Compile the software by typing:

```
make
```

##USAGE

#### Paired-end adapter trimming

```
bin/PEAT paired -1 input_fastq_pair_1 -2 input_fastq_pair_2 -o output -n thread_num -l min_gene_fragment_length
```

- input_fastq_pair_1:The paired_1 input FastQ file.
- input_fastq_pair_2:The paired_2 input FastQ file.
- output:Prefix for Output file name, stdout by default
- thread_num:Number of thread to use; if the number is larger than the core available, it will be adjusted automatically.
- min_gene_fragment_length:Minimum gene fragment length, i.e. the fragment length for reverse complement check, 30 bp by default.

#### Single-end adapter trimming

```
bin/PEAT paired -i input_fastq_file -a adapter_seq -q quality_type -o output -n thread_num
```

- input_fastq_file:The input FastQ file.
- adapter_seq:The adapter sequence, with minimum length of six characters.
- quality_type:The quality type.
- output:Output fastq file, stdout by default
- thread_num:Number of thread to use; if the number is larger than the core available, it will be adjusted automatically.

##Citing PEAT
* not yet

##Contact
	Jui-Hung Hung <juihunghung@gmail.com>
	Chou Min-Te <poi5305@gmail.com>
