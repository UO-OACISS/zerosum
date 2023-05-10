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

Sample output from a single process, 4 OpenMP program running on cores 0-7:
```
Opening log file: zs.0.log
MPI 000 - SEC 0 - Node  - PID 3281418
MPI 000 - SEC 1 - Node  - CPUs allowed: [0,1,2,3,4,5,6,7]
[Main] HWT 3281418 - CPUs allowed: [0,1,2,3,4,5,6,7]
[ZeroSum] HWT 3281419 - CPUs allowed: [0,1,2,3,4,5,6,7]
[OpenMP] HWT 3281420 - CPUs allowed: [2]
[OpenMP] HWT 3281421 - CPUs allowed: [4]
[OpenMP] HWT 3281422 - CPUs allowed: [6]

	 *** Final thread summary: ***

[Main,OpenMP] HWT 3281418 - CPUs allowed: [0]
majflt: 9,9,9,9,9,9,9,9,9
minflt: 277,334,354,8235,13651,13651,17671,17671,20870
nonvoluntary_ctxt_switches: 2,11,17,18,21,22,26
nswap: 0,0,0,0,0,0,0,0,0
processor: 1,1,0,0,0,0,0,0,0
state: R,R,R,R,R,R,R,R,R
stime: 0,0,0,2,2,2,2,2,2
utime: 0,0,0,98,197,297,396,496,596
voluntary_ctxt_switches: 54,56,56,56,56,56,56

[ZeroSum] HWT 3281419 - CPUs allowed: [0,1,2,3,4,5,6,7]
majflt: 0,0,0,0,0,0,0
minflt: 11,17,48,55,86,93,126
nonvoluntary_ctxt_switches: 0,0,0,0,0,0,0
nswap: 0,0,0,0,0,0,0
processor: 6,7,7,7,7,6,7
state: R,R,R,R,R,R,R
stime: 0,0,0,0,0,0,0
utime: 0,0,0,0,0,0,0
voluntary_ctxt_switches: 3,4,5,6,7,8,9

[OpenMP] HWT 3281420 - CPUs allowed: [2]
majflt: 0,0,0,0,0,0,0
minflt: 3,6603,10378,10378,13971,13971,15715
nonvoluntary_ctxt_switches: 2,4,4,4,4,5
nswap: 0,0,0,0,0,0,0
processor: 2,2,2,2,2,2,2
state: R,R,R,R,R,R,R
stime: 0,1,1,1,1,2,2
utime: 0,98,197,297,396,497,596
voluntary_ctxt_switches: 1,1,1,1,1,1

[OpenMP] HWT 3281421 - CPUs allowed: [4]
majflt: 0,0,0,0,0,0,0
minflt: 4,5343,9107,9107,12395,12395,13972
nonvoluntary_ctxt_switches: 3,5,6,8,8,9
nswap: 0,0,0,0,0,0,0
processor: 4,4,4,4,4,4,4
state: R,R,R,R,R,R,R
stime: 0,0,1,1,1,2,2
utime: 0,98,198,297,396,496,596
voluntary_ctxt_switches: 2,2,2,2,2,2

[OpenMP] HWT 3281422 - CPUs allowed: [6]
majflt: 0,0,0,0,0,0,0
minflt: 2,3885,7595,7595,10819,10819,12367
nonvoluntary_ctxt_switches: 1,3,4,7,8,10
nswap: 0,0,0,0,0,0,0
processor: 6,6,6,6,6,6,6
state: R,R,R,R,R,R,R
stime: 0,0,1,1,1,1,2
utime: 0,98,198,297,397,497,596
voluntary_ctxt_switches: 0,0,0,0,0,0
```
