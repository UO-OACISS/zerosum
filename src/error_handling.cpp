/*
 * Copyright (c) 2014-2021 Kevin Huck
 * Copyright (c) 2014-2021 University of Oregon
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

namespace zerosum {

void print_backtrace() {
  void *trace[32];
  size_t size, i;
  char **strings;

  size    = backtrace( trace, 32 );
  /* overwrite sigaction with caller's address */
  //trace[1] = (void *)ctx.eip;
  strings = backtrace_symbols( trace, size );

  std::cerr << std::endl;
  std::cerr << "BACKTRACE (backtrace_symbols):";
  std::cerr << std::endl;
  std::cerr << std::endl;

  char exe[256];
  int len = readlink("/proc/self/exe", exe, 256);
  if (len != -1) {
    exe[len] = '\0';
  }

  // skip the first frame, it is this handler
  for( i = 1; i < size; i++ ){
   std::cerr << strings[i] << std::endl;
  }
  std::cerr << std::endl;

}

static void custom_signal_handler(int sig) {

  int errnum = errno;

  fflush(stderr);
  std::cerr << std::endl;
  print_backtrace();
  std::cerr << std::endl;

    std::cerr << "********* Node " << ZeroSum::getInstance().getRank() <<
                " " << strsignal(sig) << " *********" << std::endl;
    std::cerr << std::endl;
    if(errnum) {
        std::cerr << "Value of errno: " << errno << std::endl;
        perror("Error printed by perror");
        std::cerr << "Error string: " << strerror( errnum ) << std::endl;
    }

  std::cerr << "***************************************";
  std::cerr << std::endl;
  std::cerr << std::endl;
  fflush(stderr);
  ZeroSum::getInstance().handleCrash();
  exit(sig);
}

std::map<int, struct sigaction> other_handlers;

static void custom_signal_handler_advanced(int sig, siginfo_t * info, void * context) {
    custom_signal_handler(sig);
    // call the old handler
    other_handlers[sig].sa_sigaction(sig, info, context);
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
    //SIGTRAP,
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
