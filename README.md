# zerosum
Utility for monitoring process, thread, OS and HW resources.


Things to check:
    - Periodically!
    - Get thread counts (done)
    - Get assigned hardware
        - process binding (done)
        - gpu binding (done)
        - openmp thread binding (done)
        - other threads (done)
    - Get utilization
        - get per-hwt utilization
        - per bound set of resources - organize into sets
    - Get contention
        - nonvoluntary_ctx_switch
        - cpustat values:
            cpu_stat->name, &cpu_stat->user, &cpu_stat->nice,
            &cpu_stat->system, &cpu_stat->idle,
            &cpu_stat->iowait, &cpu_stat->irq, &cpu_stat->softirq,
            &cpu_stat->steal, &cpu_stat->guest
        - for whole process
        - can get per thread - yes in /proc/self/task/tid/status
    - Memory usage
        - % of total
    - gpu binding
        - is it optimal? without "closest" it may not be. can we get that from lstopo?

    - check this all over time

Notes:
    - Don't want to pin progress threads. Need to identify the origin of the threads.
      MPI, HIP, CUDA threads should be allowed to float within their resource set.

Capture once:
    - Process Binding
    - GPU Binding
    - OpenMP thread ids
    - HWLOC topology

Capture periodically:
    - List of thread ids
    - For each thread:
        - get binding count, ids
        - get contention value(s)
    - For each core in process set:
        - how many threads are on this core?
            - proportional?
            - exclusive?
        - get utilization

To get backtrace of each thread:
https://github.com/albertz/openlierox/blob/0.59/src/common/Debug_GetCallstack.cpp
