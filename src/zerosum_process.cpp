/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#include "zerosum.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "utils.h"

namespace zerosum {

void ZeroSum::getProcStatus(const int rank, const int size) {
    std::string allowed_string = getCpusAllowed("/proc/self/status");
    std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
    this->process = software::Process(getpid(), rank, size, allowed_list);
    this->process.hwthreads_raw = allowed_string;
    this->process.computeNode = this->computeNode;
    logfile << this->process.toString() << std::endl;
    return;
}

}
