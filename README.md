PEAT
=========
[![Build Status](https://travis-ci.org/jhhung/PEAT.svg?branch=master)](https://travis-ci.org/jhhung/PEAT)

PEAT, a fast and highly accurate adapter-trimming algorithm, designed specifically for paired-end sequencing. PEAT requires no a priori adapter sequence and significantly outperforms other widely used adapter trimmers in both simulated and real life paired-end sequencing libraries. Moreover, in order to process single-end sequencing data, we also provide single-end trimmer in PEAT.

PEAT is released under GPLv2 with additional restriction so that is only applicable to individuals and non-profits and that any for-profit company must purchase a different license.

## INSTALL

* Only 64 bits systems are able to compile and run PEAT. 

#### Run the binary directly without installation 

Try the precompiled binaries first, most of the linux systems should be able to run PEAT without any troubles.

Please find the binary that suits your platform:

```
bin/PEAT_linux  // for centos/redhat/ubuntu/fedara/...

bin/PEAT_mac   // for MACOSX
```

Or you can find them in the release tab in this page or at this link:
https://github.com/jhhung/PEAT/releases

You can rename it to "PEAT" for your convinience.
If unfortunately, none of them works, please see below to build a binary for your box.

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

## USAGE

#### Paired-end adapter trimming

```
bin/PEAT paired -1 <file> -2 <file> -o <output> -n <num> -l <num> -r <num> -g <num> -a <num>
```

- -1 : The paired_1 input FastQ file.
- -2 : The paired_2 input FastQ file.
- -o : Prefix for Output file name, stdout by default
- -n : Number of thread to use; if the number is larger than the core available, it will be adjusted automatically.
- -l : Minimum gene fragment length, i.e. the fragment length for reverse complement check, 30 bp by default
- -r : Mismatch rate applied in first stage reverse complement scan, 0.4 by default
- -g : Mismatch rate applied in second stage gene portion check, 0.6 by default
- -a : Mismatch rate applied in second stage adapter portion check, 0.4 by default
- --qtrim : Quality trimmer; trim the last base of the reads until the mean quality value of the reads is larger than threshold
- -q : The quality type. Type any one of the following quality type indicator: ILLUMINA, PHRED, SANGER, SOLEXA. Only for the option: --qtrim
- -t : The threshold (quality value) of the quality trimmer, 30.0 by default. Only for the option: --qtrim
- --verbose : Output running process bt stderr
- --adapter_contexts : Output adapter contexts within the top ten numbers in report.txt; if you use this option, the program becomes slower.
- --adapter_min_bp : Determine the mininal length of output adapter contexts within the top 50 numbers in report.txt, 10 bp by default; Only for the option: adapter_contexts


#### Single-end adapter trimming [this funciton is adapted from: https://github.com/vsbuffalo/scythe]

```
bin/PEAT single -i <file> -a <string> -q <string> -o <output> -n <num>
```

- -i : The input FastQ file.
- -a : The adapter sequence, with minimum length of six characters.
- -q : The quality type of input FastQ file. (Acceptable quality type : 1.PHRED 2.SANGER 3.SOLEXA 4.ILLUMINA)
- -o : Output fastq file, stdout by default
- -n : Number of thread to use; if the number is larger than the core available, it will be adjusted automatically.
- --qtrim : Quality trimmer; trim the last base of the reads until the mean score is larger than threshold.
- -t :The threshold value of the quality trimmer, 30.0 by default
- --verbose : Output running process by stderr 

## Citing PEAT
Li, Y.-L., Weng, J.-C., Hsiao, C.-C., Chou, M.-T., Tseng, C.-W., & Hung, J.-H. (2015). PEAT: an intelligent and efficient paired-end sequencing adapter trimming algorithm. BMC Bioinformatics, 16(Suppl 1), S2. doi:10.1186/1471-2105-16-S1-S2

## Download the Simulation Datasets

- http://www.jhhlab.tw/Simulation_Datasets/

## Contact
	Jui-Hung Hung <juihunghung@gmail.com>

	Yun-Lung Li <obigbando@gmail.com>
	
    Jui-Cheng Weng <johnny231949@gmail.com>
	
    Chiung-Po Hsiao <restart0216s@gmail.com>
    
