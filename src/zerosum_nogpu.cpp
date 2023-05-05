/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#include "zerosum.h"
#include <cstdio>
#include <regex>
#include <iostream>
#include <vector>
#include <string>
#include "perfstubs.h"

int ZeroSum::getgpu(const int rank, const int section, const char * name) {
    char buffer[1025];
    snprintf(buffer, 1024,
        "MPI %03d - STEP %03d - SEC %d - Node %s", rank, step, section, name);
    logfile << buffer << std::endl;

    return 0;
}


