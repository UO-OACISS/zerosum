#include "zerosum.h"
#include "nvml.h"

#define NVML_CALL(call)                                                      \
do {                                                                         \
    nvmlReturn_t _status = call;                                             \
    if (_status != NVML_SUCCESS) {                                           \
        const char *errstr = nvmlErrorString(_status);                       \
        fprintf(stderr, "%s:%d: error: function %s failed with error %d: %s.\n", \
                __FILE__, __LINE__, #call, _status, errstr);                 \
        exit(-1);                                                            \
    }                                                                        \
} while (0);

#define MILLIONTH 1.0e-6 // scale to MB
#define BILLIONTH 1.0e-9 // scale to GB
#define PCIE_THROUGHPUT 1.0e-3  // to scale KB to MB
#define NVLINK_BW 1.0e-3  // to scale MB/s to GB/s
#define WATTS 1.0e-3  // scale mW to W

namespace zerosum {

std::vector<nvmlDevice_t> initialize_nvml(void);
void finalize_nvml(void);

class ScopedNVML {
private:
public:
    std::vector<nvmlDevice_t> devices;
    ScopedNVML(void) {
        devices = initialize_nvml();
    }
    ~ScopedNVML(void) {
        finalize_nvml();
    }
};

}
