import subprocess as sb
import glob
import os
import errno
import ntpath
import sys

'''
    This script creates the file include/mllib/Ml_Lib.h
    out of all files in the folder resources/mllib
    
    You dont have to understand the code just look at the input out put!
    It is just a hacked C++ code generator

'''

def main():

    #compPath = os.path.realpath(os.path.dirname(os.path.abspath(__file__)))
    if len(sys.argv) <= 1:
        print("No input file specified")
    path = sys.argv[1]
    files = glob.glob( path +"/*" )
    result = ""

    for f in files:
        p = sb.Popen("xxd -i " + f, shell=True , stdout=sb.PIPE )
        output = p.communicate()[0].decode()
        result = result + output.replace('\\n', '\n') + os.linesep + os.linesep


    libs = "unsigned char * ml_libs[] = {"
    libs_len = "unsigned int ml_libs_len[] = {"
    sep = os.linesep + "  "
    for f in files:
        f = f.replace("/","_").replace("." , "_").replace("-" , "_").replace(" " , "_")
        libs = libs + sep + f
        libs_len = libs_len + sep + f + "_len"
        sep = "," + os.linesep + "  "

    libs = libs + os.linesep + "};"
    libs_len = libs_len + os.linesep + "};"

    # Create the map that maps strings to int
    # The index can be used to to find the corresponding length and char array
    map = "static std::map<std::string, int> libNameToIndex = {"
    i = 0
    sep = os.linesep + "  "
    for f in files:
        f = ntpath.basename(f).split(".")[0]
        print ("Created lib "+f)
        st = "{ \"" + f  + "\", " + str(i) + "}"
        map = map + sep + st
        i = i + 1
        sep = "," + os.linesep + "  "
    map = map + os.linesep + "};"


    header = "#pragma once" + os.linesep + "#include <string> " + os.linesep + "#include <map>"

    result = header  + os.linesep + result + os.linesep + libs + os.linesep + libs_len + os.linesep + map


    outFilename ="include/mllib/Ml_Lib.h"
    if not os.path.exists(os.path.dirname(outFilename)):
        try:
            os.makedirs(os.path.dirname(outFilename))
        except OSError as exc: # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise



    text_file = open(outFilename, "w")
    text_file.write(result)
    text_file.close()


if __name__ == "__main__": main()
