#pragma once

#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <array>
#include "utils.h"

namespace zerosum {

namespace hardware {

class HWT {
public:
    HWT(uint32_t _id) : id(_id) {}
    HWT() = default;
    ~HWT() = default;
    uint32_t id;
    std::map<std::string, std::vector<std::string>> stat_fields;
    void updateFields(std::map<std::string, std::string> fields, uint32_t step) {
        for (auto f : fields) {
            if (stat_fields.count(f.first) == 0) {
                std::vector<std::string> v;
                stat_fields.insert(std::pair(f.first, v));
            }
            stat_fields[f.first].push_back(f.second);
        }
        //stat_fields["step"].push_back(std::to_string(step));
        UNUSED(step);
    }
    std::string strSub(std::string lhs, std::string rhs, double& total) {
        unsigned a = atol(lhs.c_str());
        unsigned b = atol(rhs.c_str());
        //double ticks = sysconf(_SC_CLK_TCK);
        //std::string tmpstr{std::to_string((a-b)/ticks)};
        unsigned result = a>b ? a-b : 0;
        total += result;
        std::string tmpstr{std::to_string(result)};
        return tmpstr;
    }
    std::string getFields() {
        std::string tmpstr;
        for (auto sf : stat_fields) {
            tmpstr += "\t";
            tmpstr += sf.first;
            tmpstr += ": ";
            bool comma = false;
            double total = 0;
            if (sf.first.compare("step") != 0) {
                auto previous = sf.second[0];
                for (auto v : sf.second) {
                    if (comma) { tmpstr += ","; }
                    tmpstr += strSub(v,previous,total);
                    comma = true;
                    previous = v;
                }
                tmpstr += " average: ";
                double average = total/(double)(std::max(size_t(1),sf.second.size()-1));
                tmpstr += std::to_string(average);
            } else {
                for (auto v : sf.second) {
                    if (comma) { tmpstr += ","; }
                    tmpstr += v;
                    comma = true;
                }
            }
            tmpstr += "\n";
        }
        return tmpstr;
    }
    std::string getSummary() {
        std::string tmpstr;
        bool comma = false;
        for (auto sf : stat_fields) {
            if (sf.first.compare("step") != 0) {
                if (comma) { tmpstr += ","; }
                tmpstr += " " + sf.first;
                tmpstr += ": ";
                double total = 0;
                auto previous = sf.second[0];
                for (auto v : sf.second) {
                    strSub(v,previous,total);
                    previous = v;
                }
                double average = total/(double)(std::max(size_t(1),sf.second.size()-1));
                char tmp[256] = {0};
                snprintf(tmp, 255, "%6.2f", average);
                tmpstr += tmp;
                comma = true;
            }
        }
        return tmpstr;
    }
};

class GPU {
public:
    GPU(std::map<std::string, std::string> props) {
        properties = props;
    }
    GPU() = default;
    ~GPU() = default;
    uint32_t id;
    std::map<std::string, std::string> properties;
    std::map<std::string, std::vector<std::string>> stat_fields;
    void updateFields(std::map<std::string, std::string> fields) {
        for (auto f : fields) {
            if (stat_fields.count(f.first) == 0) {
                std::vector<std::string> v;
                stat_fields.insert(std::pair(f.first, v));
            }
            stat_fields[f.first].push_back(f.second);
        }
    }
    std::string strSub(std::string lhs, std::string rhs, double& total) {
        unsigned a = atol(lhs.c_str());
        unsigned b = atol(rhs.c_str());
        //double ticks = sysconf(_SC_CLK_TCK);
        //std::string tmpstr{std::to_string((a-b)/ticks)};
        unsigned result = a>b ? a-b : 0;
        total += result;
        std::string tmpstr{std::to_string(result)};
        return tmpstr;
    }
    std::string getFields() {
        std::string tmpstr;
        for (auto p : properties) {
                tmpstr += "\t" + p.first;
                tmpstr += ": " + p.second + "\n";
        }
        for (auto sf : stat_fields) {
            tmpstr += "\t";
            tmpstr += sf.first;
            tmpstr += ": ";
            bool comma = false;
            double total = 0;
            if (sf.first.compare("Energy Average (J)") == 0 ||
                sf.first.compare("GFX Activity %") == 0 ||
                sf.first.compare("Memory Activity %") == 0) {
                auto previous = sf.second[0];
                for (auto v : sf.second) {
                    if (comma) { tmpstr += ","; }
                    tmpstr += strSub(v,previous,total);
                    comma = true;
                    previous = v;
                }
                tmpstr += " average: ";
                double average = total/(double)(std::max(size_t(1),sf.second.size()-1));
                tmpstr += std::to_string(average);
            } else {
                for (auto v : sf.second) {
                    if (comma) { tmpstr += ","; }
                    tmpstr += v;
                    comma = true;
                    total += atof(v.c_str());
                }
                tmpstr += " average: ";
                double average = total/(double)(std::max(size_t(1),sf.second.size()));
                tmpstr += std::to_string(average);
            }
            tmpstr += "\n";
        }
        return tmpstr;
    }
    std::string getSummary() {
        std::string tmpstr;
        for (auto sf : stat_fields) {
            tmpstr += "\t";
            tmpstr += sf.first;
            tmpstr += ": ";
            double total = 0;
            if (sf.first.compare("Energy Average (J)") == 0 ||
                sf.first.compare("GFX Activity %") == 0 ||
                sf.first.compare("Memory Activity %") == 0) {
                auto previous = sf.second[0];
                for (auto v : sf.second) {
                    strSub(v,previous,total);
                    previous = v;
                }
                double average = total/(double)(std::max(size_t(1),sf.second.size()-1));
                tmpstr += std::to_string(average);
            } else {
                for (auto v : sf.second) {
                    total += atof(v.c_str());
                }
                double average = total/(double)(std::max(size_t(1),sf.second.size()));
                tmpstr += std::to_string(average);
            }
            tmpstr += "\n";
        }
        return tmpstr;
    }
};

class ComputeNode {
public:
    ComputeNode(std::string _name) :
        name(_name), ncpus(std::thread::hardware_concurrency()) {
        hwThreads.reserve(ncpus);
        for (unsigned i = 0 ; i < ncpus ; i++) {
            hwThreads.push_back(HWT(i));
        }
    }
    ComputeNode() = default;
    ~ComputeNode() = default;
    std::string name;
    unsigned ncpus;
    std::vector<HWT> hwThreads;
    std::vector<GPU> gpus;
    void addGpu(std::vector<std::map<std::string,std::string>> props) {
        gpus.reserve(props.size());
        for (auto p : props) {
            gpus.push_back(GPU(p));
        }
    }
    void updateGPU(std::vector<std::map<std::string,std::string>> fields) {
        for (unsigned index = 0 ; index < gpus.size() ; index++) {
            gpus[index].updateFields(fields[index]);
        }
    }
    void updateFields(std::vector<std::map<std::string,std::string>> fields, uint32_t step) {
        for (unsigned index = 0 ; index < ncpus ; index++) {
            hwThreads[index].updateFields(fields[index], step);
        }
    }
    std::string toString(std::set<uint32_t> hwthreads) {
        std::string outstr{"\nHardware Summary:\n\n"};
        uint32_t index{0};
        for (auto hwt : hwThreads) {
            if (hwthreads.count(index++) > 0) {
                outstr += "CPU";
                outstr += std::to_string(hwt.id);
                outstr += "\n";
                outstr += hwt.getFields();
            }
        }
        for (auto gpu : gpus) {
            outstr += "GPU " + gpu.properties["RT_GPU_ID"] + " -";
            outstr += gpu.getFields();
            outstr += "\n";
        }
        return outstr;
    }
    std::string getSummary(std::set<uint32_t> hwthreads) {
        std::string outstr{"\nHardware Summary:\n"};
        size_t len = 3;
        for (auto hwt : hwThreads) {
            if (hwthreads.count(hwt.id) > 0) {
                std::string tmp = std::to_string(hwt.id);
                int precision = len - std::min(len,tmp.size());
                tmp.insert(0, precision, '0');
                outstr += "CPU " + tmp + " -";
                outstr += hwt.getSummary();
                outstr += "\n";
            }
        }
        for (auto gpu : gpus) {
            outstr += "GPU " + gpu.properties["RT_GPU_ID"] + " -";
            outstr += gpu.getSummary();
            outstr += "\n";
        }
        outstr += "\nOther Hardware:\n";
        for (auto hwt : hwThreads) {
            if (hwthreads.count(hwt.id) == 0) {
                std::string tmp = std::to_string(hwt.id);
                int precision = len - std::min(len,tmp.size());
                tmp.insert(0, precision, '0');
                outstr += "CPU " + tmp + " -";
                outstr += hwt.getSummary();
                outstr += "\n";
            }
        }
        return outstr;
    }
};

} // namespace hardware

namespace software {

enum ThreadType { Main = 0x1,
                  ZeroSum = 0x2,
                  OpenMP = 0x4,
                  Other = 0x8 };

class LWP {
public:
    LWP(uint32_t _id, std::vector<uint32_t> allowed_hwt,
        std::map<std::string, std::string> fields,
        ThreadType _type = Other) : id(_id), type(_type) {
        for (auto t : allowed_hwt) {
            hwthreads.insert(t);
        }
        extractFields(fields);
    }
    //LWP(uint32_t _id, ThreadType _type) : id(_id), type(_type) { }
    LWP() = default;
    ~LWP() = default;
    LWP(const LWP& lwp) = default;
    /*
    LWP(const LWP& lwp) {
        id = lwp.id;
        type = lwp.type;
        type_id = lwp.type_id;
        hwthreads = lwp.hwthreads;
        stat_fields = lwp.stat_fields;
    }
    */
    uint32_t id;
    uint32_t type;
    uint32_t type_id;
    // The cores this lwp can run on
    std::set<uint32_t> hwthreads;
    // The relevant /proc/self/task/tid/stat fields
    std::map<std::string, std::vector<std::string>> stat_fields;
    void update(std::vector<uint32_t> allowed_hwt, std::map<std::string, std::string> fields,
        ThreadType _type) {
        hwthreads.clear();
        for (auto t : allowed_hwt) {
            hwthreads.insert(t);
        }
        if (_type != Other) {
            type |= _type;
        }
        extractFields(fields);
    }
    void extractFields(std::map<std::string, std::string> fields) {
        for (auto f : fields) {
            if (stat_fields.count(f.first) == 0) {
                std::vector<std::string> v;
                stat_fields.insert(std::pair(f.first, v));
            }
            stat_fields[f.first].push_back(f.second);

        }
    }
    std::string strSub(std::string lhs, std::string rhs) {
        unsigned a = atol(lhs.c_str());
        unsigned b = atol(rhs.c_str());
        unsigned result = a>b ? a-b : 0;
        std::string tmpstr{std::to_string(result)};
        return tmpstr;
    }
    std::string getFields() {
        std::string tmpstr;
        for (auto sf : stat_fields) {
            tmpstr += sf.first;
            tmpstr += ": ";
            bool comma = false;
            if (sf.first.compare("state") != 0 &&
                sf.first.compare("step") != 0 &&
                sf.first.compare("processor") != 0) {
                auto previous = sf.second[0];
                for (auto v : sf.second) {
                    if (comma) { tmpstr += ","; }
                    tmpstr += strSub(v,previous);
                    comma = true;
                    previous = v;
                }
            } else {
                for (auto v : sf.second) {
                    if (comma) { tmpstr += ","; }
                    tmpstr += v;
                    comma = true;
                }
            }
            tmpstr += "\n";
        }
        return tmpstr;
    }
    std::string typeToString(void) {
        std::string sType;
        if (type & Main) {
            sType = "Main";
        }
        if (type & ZeroSum) {
            sType = "ZeroSum";
        }
        if (type & OpenMP) {
            if (sType.size() > 0) { sType += ","; }
            sType += "OpenMP";
        }
        if (type & Other) {
            if (sType.size() > 0) { sType += ","; }
            sType += "Other";
        }
        size_t n = 11;
        size_t precision = n - std::min(n, sType.size());
        sType.insert(0, precision, ' ');
        return sType;
    }

    std::string toString(bool shutdown = false) {
        char buffer[1025];
        std::string sType = typeToString();
        snprintf(buffer, 1024,
            "New Thread: [%s] LWP %d - CPUs allowed: [%s]\n",
            sType.c_str(), id, ::zerosum::toString(hwthreads).c_str());
        std::string outstr{buffer};
        if (shutdown) {
            outstr += getFields();
            outstr += "\n";
        }
        return outstr;
    }
    std::string getSummary(void) {
        std::string tmpstr;
        tmpstr += "LWP " + std::to_string(id) + ": " + typeToString() + " -";
        for (auto sf : stat_fields) {
            if (sf.first.compare("utime") == 0 ||
                sf.first.compare("stime") == 0) {
                tmpstr += " " + sf.first;
                tmpstr += ": ";
                double total = std::stod(sf.second.back());
                double average = total/(double)(std::max(size_t(1),sf.second.size()-1));
                char tmp[256] = {0};
                snprintf(tmp, 255, "%6.2f", average);
                tmpstr += tmp;
                tmpstr += ",";
            }
        }
        for (auto sf : stat_fields) {
            if (sf.first.compare("nonvoluntary_ctxt_switches") == 0) {
                tmpstr += " nv_ctx";
                tmpstr += ": ";
                size_t total = std::stol(sf.second.back());
                char tmp[256] = {0};
                snprintf(tmp, 255, "%5lu", total);
                tmpstr += tmp;
                tmpstr += ",";
            }
            if (sf.first.compare("voluntary_ctxt_switches") == 0) {
                tmpstr += " ctx";
                tmpstr += ": ";
                size_t total = std::stol(sf.second.back());
                char tmp[256] = {0};
                snprintf(tmp, 255, "%5lu", total);
                tmpstr += tmp;
                tmpstr += ",";
            }
        }
        tmpstr += " CPUs allowed: [" + ::zerosum::toString(hwthreads) + "]";
        return tmpstr;
    }
};

inline bool operator<(const LWP& lhs, const LWP& rhs) {
  return (lhs.id < rhs.id);
}

/* The linux process */
class Process {
public:
    Process(uint32_t _id, uint32_t _rank, uint32_t _size,
        std::map<std::string, std::string> fields,
        std::vector<uint32_t> allowed_hwt) : id(_id), rank(_rank), size(_size) {
        for (auto t : allowed_hwt) {
            hwthreads.insert(t);
        }
        // pid should be the same as tid, but just in case...
        threads.insert(std::pair(id, LWP(gettid(), allowed_hwt, fields, ThreadType::Main)));
    }
    Process() = default;
    ~Process() = default;
    void add(uint32_t tid, std::vector<uint32_t> allowed_hwt,
        std::map<std::string, std::string> fields, ThreadType type = Other) {
        if (threads.count(tid) == 0) {
            threads.insert(std::pair(tid, LWP(tid, allowed_hwt, fields, type)));
        } else {
            threads[tid].update(allowed_hwt, fields, type);
        }
    }
    // disabled copy constructor
    //Process(const Process&) = default;
    uint32_t id; // The process ID
    uint32_t rank; // The MPI rank
    uint32_t size; // The MPI size
    std::set<uint32_t> hwthreads; // The cores this process can run on
    std::string hwthreads_raw;
    std::map<uint32_t, LWP> threads; // The set of lightweight OS threads of this process
    // The compute node we're running on
    hardware::ComputeNode* computeNode;
    std::map<std::string,std::string> environment;

    uint32_t getMaxHWT(void) {
        // this is an iterator, so return the element
        return *(hwthreads.rbegin());
    }

    bool contains(uint32_t hwt) {
        return (threads.count(hwt) > 0);
    }

    std::string toString(void) {
        char buffer[1025];
        snprintf(buffer, 1024,
            "MPI %03d - SEC %d - Node %s - PID %d\n",
            rank, 0, computeNode->name.c_str(), id);
        std::string tmpstr(buffer);

        std::stringstream ss;
        bool comma = false;
        for (auto t : hwthreads) {
            if (comma) ss << ",";
            ss << t;
            comma = true;
        }
        std::string discrete{ss.str()};
        snprintf(buffer, 1024,
            "MPI %03d - SEC %d - Node %s - CPUs allowed: [%s]\n",
            rank, 1, computeNode->name.c_str(), discrete.c_str());
        tmpstr += buffer;
        //PERFSTUBS_METADATA("CPUs allowed", discrete.c_str());
        for (auto env : environment) {
            tmpstr += env.first + " = " + env.second + "\n";
        }
        return tmpstr;
    }
    std::string logThreads(bool shutdown = false) {
        std::string tmpstr;
        static size_t nthreads{0};
        if (shutdown) {
            tmpstr += "\n\t *** Final thread summary: *** \n\n";
            nthreads = 0;
        }
        /* TODO: Need to convert utime and stime by dividing by sysconf(_SC_CLK_TCK) */
        if (nthreads < threads.size()) {
            size_t index{0};
            for (auto t : threads) {
                if (index++ >= nthreads) {
                    tmpstr += t.second.toString(shutdown);
                }
            }
        }
        nthreads = threads.size();
        return tmpstr;
    }
    std::string getSummary(void) {
        std::string tmpstr{"\nProcess Summary:\n"};
        // print process data
        std::stringstream ss;
        bool comma = false;
        for (auto t : hwthreads) {
            if (comma) ss << ",";
            ss << t;
            comma = true;
        }
        std::string discrete{ss.str()};
        char buffer[1025];
        snprintf(buffer, 1024,
            "MPI %03d - PID %d - Node %s - CPUs allowed: [%s]\n",
            rank, id, computeNode->name.c_str(), discrete.c_str());
        tmpstr += buffer;

        // print total threads
        tmpstr += "\nLWP (thread) Summary:\n";
        for (auto t : threads) {
            tmpstr += t.second.getSummary();
            tmpstr += "\n";
        }
        // print assigned cores
        tmpstr += computeNode->getSummary(hwthreads);
        return tmpstr;
    }
};

} // namespace software

} // namespace zerosum