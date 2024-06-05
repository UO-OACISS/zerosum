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

#include <string.h>
#include <dirent.h>
#include <signal.h>
#include "zerosum.h"
#include "utils.h"
#include <dlfcn.h>
#include <unordered_map>

typedef int (*pthread_mutex_lock_p)(pthread_mutex_t *mutex);
typedef int (*pthread_mutex_trylock_p)(pthread_mutex_t *mutex);
typedef int (*pthread_cond_wait_p)(pthread_cond_t *cond, pthread_mutex_t *mutex);
typedef int (*pthread_cond_timedwait_p)(pthread_cond_t *cond,
    pthread_mutex_t *mutex, const struct timespec *abstime);

#define RESET_DLERROR() dlerror()
#define CHECK_DLERROR() { \
  char const * err = dlerror(); \
  if (err) { \
    printf("Error getting %s handle: %s\n", name, err); \
    fflush(stdout); \
    exit(1); \
  } \
}

static void * get_system_function_handle(char const * name, void * caller) {
    void * handle;
    // Reset error pointer
    RESET_DLERROR();
    // Attempt to get the function handle
    handle = dlsym(RTLD_NEXT, name);
    // Detect errors
    CHECK_DLERROR();
    // Prevent recursion if more than one wrapping approach has been loaded.
    // This happens because we support wrapping pthreads three ways at once:
    // #defines in Profiler.h, -Wl,-wrap on the link line, and LD_PRELOAD.
    if (handle == caller) {
        RESET_DLERROR();
        void * syms = dlopen(NULL, RTLD_NOW);
        CHECK_DLERROR();
        do {
            RESET_DLERROR();
            handle = dlsym(syms, name);
            CHECK_DLERROR();
        } while (handle == caller);
    }
    return handle;
}

typedef struct thread_counters {
    std::atomic<size_t> locks;
    std::atomic<size_t> trylocks;
    std::atomic<size_t> waits;
    std::atomic<size_t> timedwaits;
} thread_counters_t;

std::mutex& mapMutex() {
    static std::mutex mtx;
    return mtx;
}

std::unordered_map<uint32_t,thread_counters_t*>& getCounterMap() {
    static std::unordered_map<uint32_t,thread_counters_t*> _theMap;
    return _theMap;
}

thread_counters_t* getMyCounters(uint32_t tid) {
    // lock the map
    std::lock_guard l{mapMutex()};
    auto tmp = getCounterMap().find(tid);
    if (tmp == getCounterMap().end()) {
        getCounterMap()[tid] = new thread_counters_t;
    }
    return getCounterMap()[tid];
}

namespace zerosum {

int ZeroSum::getpthreads() {
    std::string tmpstr;
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/proc/self/task");
    static bool verbose{getVerbose()};
    static bool deadlock{parseBool("ZS_DETECT_DEADLOCK",false)};
    static int deadlock_duration{parseInt("ZS_DEADLOCK_DURATION",5)};
    static int deadlock_detected_seconds = 0;
    if (dp != NULL)
    {
        size_t running = 0;
        while ((ep = readdir (dp)) != NULL) {
            if (strncmp(ep->d_name, ".", 1) == 0) continue;
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
            tmpstr = tmpstr + ep->d_name;
            uint32_t lwp = atol(ep->d_name);
            int nhwthr = 0;
            std::string tmpstr2;
            std::vector<uint32_t> allowed_list_posix =
                getAffinityList(lwp, computeNode.ncpus, nhwthr, tmpstr2);
            // also want to read /proc/<pid>/task/<tid>/status!
            std::string filename = "/proc/self/task/";
            filename += ep->d_name;
            filename += "/stat";
            auto fields = getThreadStat(filename.c_str());
            bool isMain{atol(ep->d_name) == process.id};
            if (isRunning(fields, ep->d_name, isMain)) { running++; }
            filename += "us";
            std::string allowed_string = getCpusAllowed(filename.c_str());
            std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
            getThreadStatus(filename.c_str(), fields);
            //std::cout << filename << " : " << allowed_string << std::endl;
            auto counters = getMyCounters(lwp);
            fields.insert(std::pair("pthread lock calls",std::to_string(counters->locks)));
            fields.insert(std::pair("pthread trylock calls",std::to_string(counters->trylocks)));
            fields.insert(std::pair("pthread wait calls",std::to_string(counters->waits)));
            fields.insert(std::pair("pthread timedwait calls",std::to_string(counters->timedwaits)));
            fields.insert(std::pair("step",std::to_string(step)));
            if (lwp == async_tid) {
                this->process.add(lwp, allowed_list, fields, software::ThreadType::ZeroSum);
            } else {
                this->process.add(lwp, allowed_list, fields);
            }
        }
        (void) closedir (dp);
        // if there is only one running thread (this one, belonging to ZS), be concerned...
        if (deadlock) {
            if (verbose) {
                ZeroSum::getInstance().getLogfile() << running << " threads running" << std::endl;
            }
            if (running <= 1) {
                static int period{parseInt("ZS_PERIOD", 1)};
                deadlock_detected_seconds++;
                ZeroSum::getInstance().getLogfile() << "All threads sleeping for " <<
                    deadlock_detected_seconds*period << " seconds...?" << std::endl;
            } else {
                deadlock_detected_seconds = 0;
            }
            if (deadlock_detected_seconds >= deadlock_duration) {
                ZeroSum::getInstance().getLogfile() << "Deadlock detected! Aborting!" << std::endl;
                ZeroSum::getInstance().getLogfile() << "Thread " << gettid() << " signalling " << this->process.id << std::endl;
                finalizeLog();
                pthread_kill(this->process.id, SIGQUIT);
            }
        }
    }
    return 0;
}

} // namespace zerosum

extern "C" {

int pthread_mutex_lock(pthread_mutex_t *mutex) {
    static pthread_mutex_lock_p _pthread_mutex_lock =
        (pthread_mutex_lock_p)get_system_function_handle(
        "pthread_mutex_lock", (void*)pthread_mutex_lock);
    // prevent recursion
    static thread_local bool inWrapper{false};
    if (inWrapper) {
        return _pthread_mutex_lock(mutex);
    }
    // do the work to increment the counter
    inWrapper = true;
    static thread_local thread_counters_t* counters = getMyCounters(gettid());
    counters->locks++;
    int ret = _pthread_mutex_lock(mutex);
    inWrapper = false;
    return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
    static pthread_mutex_trylock_p _pthread_mutex_trylock =
        (pthread_mutex_trylock_p)get_system_function_handle(
        "pthread_mutex_trylock", (void*)pthread_mutex_trylock);
    // prevent recursion
    static thread_local bool inWrapper{false};
    if (inWrapper) {
        return _pthread_mutex_trylock(mutex);
    }
    // do the work to increment the counter
    inWrapper = true;
    static thread_local thread_counters_t* counters = getMyCounters(gettid());
    counters->trylocks++;
    int ret = _pthread_mutex_trylock(mutex);
    inWrapper = false;
    return ret;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    static pthread_cond_wait_p _pthread_cond_wait =
        (pthread_cond_wait_p)get_system_function_handle(
        "pthread_cond_wait", (void*)pthread_cond_wait);
    // prevent recursion
    static thread_local bool inWrapper{false};
    if (inWrapper) {
        return _pthread_cond_wait(cond, mutex);
    }
    // do the work to increment the counter
    inWrapper = true;
    static thread_local thread_counters_t* counters = getMyCounters(gettid());
    counters->waits++;
    int ret = _pthread_cond_wait(cond, mutex);
    inWrapper = false;
    return ret;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
    const struct timespec *abstime) {
    static pthread_cond_timedwait_p _pthread_cond_timedwait =
        (pthread_cond_timedwait_p)get_system_function_handle(
        "pthread_cond_timedwait", (void*)pthread_cond_timedwait);
    // prevent recursion
    static thread_local bool inWrapper{false};
    if (inWrapper) {
        return _pthread_cond_timedwait(cond, mutex, abstime);
    }
    // do the work to increment the counter
    inWrapper = true;
    static thread_local thread_counters_t* counters = getMyCounters(gettid());
    counters->timedwaits++;
    int ret = _pthread_cond_timedwait(cond, mutex, abstime);
    inWrapper = false;
    return ret;
}

} // extern "C"
