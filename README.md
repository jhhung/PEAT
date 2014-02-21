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
bin/PEAT paired -1 <file> -2 <file> -o <output> -n <num> -l <num> -r <num> -g <num> -a <num>
```

- -1 : The paired_1 input FastQ file.
- -2 : The paired_2 input FastQ file.
- -o : Prefix for Output file name, stdout by default
- -n : Number of thread to use; if the number is larger than the core available, it will be adjusted automatically.
- -l : Minimum gene fragment length, i.e. the fragment length for reverse complement check, 30 bp by default
- -r : Mismatch rate applied in first stage reverse complement scan, 0.3 by default
- -g : Mismatch rate applied in second stage gene portion check, 0.6 by default
- -a : Mismatch rate applied in second stage adapter portion check, 0.4 by default

#### Single-end adapter trimming

```
bin/PEAT single -i <file> -a <string> -q <string> -o <output> -n <num>
```

- -i : The input FastQ file.
- -a : The adapter sequence, with minimum length of six characters.
- -q : The quality type of input FastQ file.
- -o : Output fastq file, stdout by default
- -n : Number of thread to use; if the number is larger than the core available, it will be adjusted automatically.

##Citing PEAT
* not yet

##Contact
	Jui-Hung Hung <juihunghung@gmail.com>
	Yun-Lung Li <obigbando@gmail.com>
	Jui-Cheng Weng <johnny231949@gmail.com>
