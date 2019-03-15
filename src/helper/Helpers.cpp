//
// Created by matthias on 19.09.18.
//

#include "helper/Helpers.h"


/**
 * Executes a command in the command line
 * @param cmd command to execute
 * @return Returns the outpur of the executed command.
 */
std::stringstream exec(std::string cmd) {
    std::array<char, 1024> buffer;
    std::stringstream result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 1024, pipe.get()) != nullptr)
            result << buffer.data();
    }
    return result;
}
