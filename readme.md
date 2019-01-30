# ML Compiler
A Compiler that compiles a declarative language for machine learning in either Hyperscript, Postgresql, or Python



---
## Dependencies



Project generation and build:

* (CMake and Make) or only Make

Compiler with C++11 support:

* g++ 5.5.0

Libraries:

* [ANTLR4](http://www.antlr.org/)
* csvkit (install by "sudo apt install python3-csvkit")







---
## Building


#### Using Cmake and Make (recommended):
```sh
mkdir build && cd build
cmake ..
make
```

Stores executable in bin folder.
	
---
## Running the Compiler



Run the following to run the compiler:
```sh
./bin/compiler <inputFile> <outputFile> <targetLanguage> <flags>
```
The target language can be either postgres, hyper, or python.
"flags" can consist of either "buildcsv", "usemod" or both.

Since Hyper and Postgres can only load csv files in the correct format, the flag "buildcsv" allows the compiler the create a new modifeid csv that can be processed by the database system. (The new csv file created has the file ending .ml.csv)

To allow the compiler to create code that uses the newly created csv file that was created by using the "buildcsv" flag.

Notice, you may use the flag "buildcsv" only when you compil code the first time or have made any changes to the csv file, since using the "buildcsv" takes some time to precess the csv file.





---    
## Installing ANTLR4



To install ANTRL4 run the following commands in terminal:
```sh
wget http://www.antlr.org/download/antlr4-cpp-runtime-4.7.1-source.zip
wget http://www.antlr.org/download/antlr-4.7.1-complete.jar
sudo mv antlr-4.7.1-complete.jar /usr/local/lib/antlr-4.7.1-complete.jar
sudo apt-get install uuid-dev -y
sudo apt install cmake -y
unzip antlr4-cpp-runtime-4.7.1-source.zip -d antlr4-cpp-runtime
cd antlr4-cpp-runtime && mkdir build && mkdir run && cd build
cmake ..
make -j8
sudo make install
LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH
sudo ldconfig -v
```




