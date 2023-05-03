/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#include <cstdio>
#include "zerosum.h"

int ZeroSum::getgpu(const int rank, const int section, const char * name) {
    printf("MPI %03d - SEC %d - Node %s\n", rank, section, name);
    return 0;
}
