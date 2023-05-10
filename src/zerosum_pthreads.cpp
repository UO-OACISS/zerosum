/**********************************************************
  "Hello World"-type program to test different srun layouts.

  Written by Tom Papatheodore
 **********************************************************/

#include <string.h>
#include <dirent.h>
#include "zerosum.h"
#include "utils.h"

namespace zerosum {

int ZeroSum::getpthreads() {
    std::string tmpstr;
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/proc/self/task");
    if (dp != NULL)
    {
        while ((ep = readdir (dp)) != NULL) {
            if (strncmp(ep->d_name, ".", 1) == 0) continue;
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
            tmpstr = tmpstr + ep->d_name;
            uint32_t lwp = atol(ep->d_name);
            int nhwthr = 0;
            std::string tmpstr2;
            std::vector<uint32_t> allowed_list =
                getAffinityList(lwp, computeNode.ncpus, nhwthr, tmpstr2);
            // also want to read /proc/<pid>/task/<tid>/status!
               std::string filename = "/proc/self/task/";
               filename += ep->d_name;
               filename += "/stat";
               //std::string allowed_string = getCpusAllowed(filename.c_str());
               auto fields = getThreadStat(filename.c_str());
               filename += "us";
               getThreadStatus(filename.c_str(), fields);
               //std::cout << filename << " : " << allowed_string << std::endl;
            if (lwp == async_tid) {
                this->process.add(lwp, allowed_list, fields, software::ThreadType::ZeroSum);
            } else {
                this->process.add(lwp, allowed_list, fields);
            }
        }
        (void) closedir (dp);
    }
    return 0;
}

} // namespace zerosum
