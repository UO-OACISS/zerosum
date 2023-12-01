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

#include <string>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#include <sys/ucontext.h>
#include <errno.h>
#include <string.h>
#include <regex>
#include "utils.h"
#include "error_handling.h"
#include "zerosum.h"
#ifdef USE_MPI
#include "mpi.h"
#endif

namespace zerosum {

void print_backtrace() {
  void *trace[32];
  size_t size, i;
  char **strings;
  std::stringstream ss;

  size    = backtrace( trace, 32 );
  /* overwrite sigaction with caller's address */
  //trace[1] = (void *)ctx.eip;
  strings = backtrace_symbols( trace, size );

  ss << "\nBACKTRACE (backtrace_symbols):\n\n";

  char exe[256];
  int len = readlink("/proc/self/exe", exe, 256);
  if (len != -1) {
    exe[len] = '\0';
  }

  // skip the first frame, it is this handler
  for( i = 1; i < size; i++ ){
   ss << strings[i] << "\n";
  }
  ZeroSum::getInstance().getLogfile() << ss.str() <<std::endl;
  std::cerr << ss.rdbuf() << std::endl;
}

static void custom_signal_handler(int sig) {
  std::stringstream ss;
  int errnum = errno;

  fflush(stderr);
  print_backtrace();

    ss << "\n********* Node " << ZeroSum::getInstance().getRank() <<
                " " << strsignal(sig) << " *********\n\n";
    if(errnum) {
        ss << "\nValue of errno: " << errno << std::endl;
        perror("Error printed by perror");
        ss << "\nError string: " << strerror( errnum );
    }

  ss << "\n***************************************\n\n";
  ZeroSum::getInstance().getLogfile() << ss.str() <<std::endl;
  std::cerr << ss.rdbuf() << std::endl;
  fflush(stderr);
  ZeroSum::getInstance().handleCrash();
#ifdef USE_MPI
  MPI_Abort(MPI_COMM_WORLD, errnum);
#endif
  exit(sig);
}

std::map<int, struct sigaction> other_handlers;

static void custom_signal_handler_advanced(int sig, siginfo_t * info, void * context) {
    custom_signal_handler(sig);
    // call the old handler
    other_handlers[sig].sa_sigaction(sig, info, context);
}

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

void block_signal() {
    // block SIGQUIT in the calling thread (our async thread)
    // see: https://linux.die.net/man/3/pthread_sigmask
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    int rc = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (rc != 0) handle_error_en(rc, "pthread_sigmask");
}

int register_signal_handler() {
    //std::cout << "ZeroSum signal handler registering..." << std::endl;
    static bool once{false};
    if (once) return 0;
    struct sigaction act;
    struct sigaction old;
    memset(&act, 0, sizeof(act));
    memset(&old, 0, sizeof(old));
    /* call backtrace once, to "prime the pump" so calling backtrace
    * during a async-signal-safe handler doesn't allocate memory to
    * dynamically load glibc */
    void* dummy = NULL;
    backtrace(&dummy, 1);

    sigemptyset(&act.sa_mask);
    std::array<int,13> mysignals = {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGILL,
        SIGIOT,
        SIGBUS,
        SIGFPE,
        SIGKILL,
        SIGSEGV,
        SIGABRT,
        SIGTERM,
        SIGXCPU,
        SIGXFSZ
    };
    //act.sa_handler = custom_signal_handler;
    //act.sa_flags = 0;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    act.sa_sigaction = custom_signal_handler_advanced;
    for (auto s : mysignals) {
        sigaction(s, &act, &old);
        other_handlers[s] = old;
    }
    once = true;
    //std::cout << "ZeroSum signal handler registered!" << std::endl;
    return 0;
}

void test_signal_handler() {
  custom_signal_handler(1);
}

}
