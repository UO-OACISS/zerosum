#include "zerosum.h"
#include "rocm_smi/rocm_smi.h"

#define RSMI_CALL(call)                                                      \
do {                                                                         \
    rsmi_status_t _status = call;                                            \
    if (_status != RSMI_STATUS_SUCCESS) {                                    \
        const char *errstr;                                                  \
        if (rsmi_status_string(_status, &errstr) == RSMI_STATUS_SUCCESS) {   \
        fprintf(stderr, "%s:%d: error: function %s failed with error %d: %s.\n", \
                __FILE__, __LINE__, #call, _status, errstr);                 \
        fprintf(stderr, "\nError: ROCm SMI call failed\n");                 \
        exit(0);                                                                             \
        }                                                                    \
    }                                                                        \
} while (0);

namespace zerosum {

std::vector<uint32_t> initialize_rocm_smi(void);
void finalize_rocm_smi(void);

class ScopedRSMI {
private:
public:
    std::vector<uint32_t> devices;
    ScopedRSMI(void) {
        devices = initialize_rocm_smi();
    }
    ~ScopedRSMI(void) {
        finalize_rocm_smi();
    }
};

}
