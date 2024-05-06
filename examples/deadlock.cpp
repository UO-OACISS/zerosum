/*
 * MIT License
 *
 * Copyright (c) 2023 University of Oregon, Kevin Huck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdlib>
#include <cstdio>
#include <thread>
#include <mutex>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#ifdef USE_MPI
#include <mpi.h>
#define MPI_INIT  {int provided ; MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);}
#define MPI_FINI  MPI_Finalize();
// this barrier is intentionally broken, it will cause live-lock
#define MPI_BARRIER if (rank > 0) MPI_Barrier(MPI_COMM_WORLD);
#define MPI_COMM_RANK MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#define UNUSED(expr)
#else
#define MPI_INIT
#define MPI_FINI
#define MPI_BARRIER
#define MPI_COMM_RANK
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

std::mutex m1;

void worker_function(int rank) {
    sleep(1);
    std::unique_lock l1{m1};
    std::stringstream ss;
    ss << "Hello, I am thread " << gettid() << " from rank " << rank;
    std::cout << ss.rdbuf() << std::endl;
    sleep(9);
}

int main(int argc, char *argv[]){

    int rank{0};
    /* Set up MPI */
    MPI_INIT
    MPI_COMM_RANK
    UNUSED(argc);
    UNUSED(argv);

    /* do deadlock */
    std::thread t1(worker_function, rank);
    std::thread t2(worker_function, rank);
    std::stringstream ss;
    ss << "Hello, I am the main thread " << gettid() << " from rank " << rank;
    std::cout << ss.rdbuf() << std::endl;
    if (rank > 0) sleep(5);
    t1.join();
    t2.join();
    std::cout << "Entering deadlocked barrier..." << std::endl;
    MPI_BARRIER

    /* Finalize MPI */
    MPI_FINI

    return 0;
}
