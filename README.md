# zerosum
Utility for monitoring process, thread, OS and HW resources.

```
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
```

To get backtrace of each thread:
https://github.com/albertz/openlierox/blob/0.59/src/common/Debug_GetCallstack.cpp
This could be useful to determine library source of thread, if needed.

Sample output from the first MPI rank of an 8 process job on Frontier (see [job-frontier.sh](job-frontier.sh)):
```
Duration of execution: 12.4312 s

Process Summary:
MPI 000 - PID 23319 - Node frontier00255 - CPUs allowed: [1,2,3,4,5,6,7,65,66,67,68,69,70,71]

LWP (thread) Summary:
LWP 23319: Main,OpenMP - stime:   2.38, utime:  90.69, nv_ctx:     1, ctx:  1537, CPUs allowed: [1,65]
LWP 23324:     ZeroSum - stime:   0.27, utime:   0.18, nv_ctx:     0, ctx:    25, CPUs allowed: [71]
LWP 23332:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     0, ctx:     5, CPUs allowed: [1,65]
LWP 23341:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     0, ctx:     5, CPUs allowed: [2,66]
LWP 23349:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     1, ctx:     4, CPUs allowed: [2,66]
LWP 23355:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     0, ctx:     4, CPUs allowed: [3,67]
LWP 23362:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     1, ctx:     3, CPUs allowed: [3,67]
LWP 23369:      OpenMP - stime:   0.33, utime:  98.08, nv_ctx: 11773, ctx:     2, CPUs allowed: [4,68]
LWP 23377:      OpenMP - stime:   0.33, utime:  98.08, nv_ctx: 11773, ctx:     3, CPUs allowed: [4,68]
LWP 23385:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     1, ctx:     3, CPUs allowed: [5,69]
LWP 23393:      OpenMP - stime:   0.33, utime:  98.92, nv_ctx:     1, ctx:     3, CPUs allowed: [5,69]
LWP 23400:      OpenMP - stime:   0.25, utime:  99.00, nv_ctx:     1, ctx:     3, CPUs allowed: [6,70]
LWP 23408:      OpenMP - stime:   0.25, utime:  99.00, nv_ctx:     0, ctx:     3, CPUs allowed: [6,70]
LWP 23416:      OpenMP - stime:   0.25, utime:  98.42, nv_ctx:    26, ctx:     3, CPUs allowed: [7,71]
LWP 23423:      OpenMP - stime:   0.25, utime:  99.00, nv_ctx:     1, ctx:     3, CPUs allowed: [7,71]
LWP 23453:       Other - stime:   0.00, utime:   0.00, nv_ctx:     0, ctx:     3, CPUs allowed: [1,2,3,4,5,6,7,9,10,11,12,13,14,15,17,18,19,20,21,22,23,25,26,27,28,29,30,31,33,34,35,36,37,38,39,41,42,43,44,45,46,47,49,50,51,52,53,54,55,57,58,59,60,61,62,63,65,66,67,68,69,70,71,73,74,75,76,77,78,79,81,82,83,84,85,86,87,89,90,91,92,93,94,95,97,98,99,100,101,102,103,105,106,107,108,109,110,111,113,114,115,116,117,118,119,121,122,123,124,125,126,127]

Hardware Summary:
CPU 001 - idle:   0.00, system:   0.18, user:  99.64
CPU 002 - idle:   0.00, system:   0.09, user:  99.64
CPU 003 - idle:   0.00, system:   0.09, user:  99.64
CPU 004 - idle:   0.00, system:   0.09, user:  99.64
CPU 005 - idle:   0.00, system:   0.09, user:  99.64
CPU 006 - idle:   0.00, system:   0.09, user:  99.64
CPU 007 - idle:   0.00, system:   0.09, user:  99.64
CPU 065 - idle:   0.00, system:   0.00, user:  99.73
CPU 066 - idle:   0.00, system:   0.09, user:  99.64
CPU 067 - idle:   0.00, system:   0.09, user:  99.64
CPU 068 - idle:   0.00, system:   0.09, user:  99.64
CPU 069 - idle:   0.00, system:   0.09, user:  99.64
CPU 070 - idle:   0.00, system:   0.09, user:  99.64
CPU 071 - idle:   0.00, system:   0.36, user:  99.45
```
