#include <set>
#include <thread>
#include "mpi.h"

class ZeroSum {
public:
    static ZeroSum& getInstance() {
        static ZeroSum instance;
        return instance;
    }

    // Other public member functions...
    void shutdown(void);

private:
    ZeroSum();
    ~ZeroSum() = default;
    ZeroSum(const ZeroSum&) = delete;
    ZeroSum& operator=(const ZeroSum&) = delete;
    ZeroSum(ZeroSum&&) = delete;
    ZeroSum& operator=(ZeroSum&&) = delete;

    // member variables
    std::thread worker;
    int section;
    std::set<long> tids;
    int ncpus;
    bool working;
    int size;
    int rank;
    char name[MPI_MAX_PROCESSOR_NAME];

    // Other private member variables and functions...
    int getgpu(const int rank, const int section, const char * name);
    int getopenmp(const int rank, const int section, const int ncpus, std::set<long>& tids);
    int getpthreads(const int rank, const int section, const int ncpus, std::set<long>& tids);
    void getProcStatus(const int section);
    void threadedFunction(void);
    bool doOnce(void);
    void doPeriodic(void);
};


