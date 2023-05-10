#pragma once

#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <array>
#include "utils.h"

namespace zerosum { namespace hardware {

class ComputeNode {
public:
    ComputeNode(std::string _name) :
        name(_name), ncpus(std::thread::hardware_concurrency()) {}
    ComputeNode() = default;
    ~ComputeNode() = default;
    std::string name;
    unsigned ncpus;
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
    std::string getFields() {
        std::string tmpstr;
        for (auto sf : stat_fields) {
            tmpstr += sf.first;
            tmpstr += ": ";
            bool comma = false;
            for (auto v : sf.second) {
                if (comma) { tmpstr += ","; }
                tmpstr += v;
                comma = true;
            }
            tmpstr += "\n";
        }
        return tmpstr;
    }
    std::string toString(bool shutdown = false) {
        char buffer[1025];
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

        snprintf(buffer, 1024,
            "[%s] HWT %d - CPUs allowed: [%s]\n",
            sType.c_str(), id, ::zerosum::toString(hwthreads).c_str());
        std::string outstr{buffer};
        if (shutdown) {
            outstr += getFields();
            outstr += "\n";
        }
        return outstr;
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
    hardware::ComputeNode computeNode;

    bool contains(uint32_t hwt) {
        return (threads.count(hwt) > 0);
    }

    std::string toString(void) {
        char buffer[1025];
        snprintf(buffer, 1024,
            "MPI %03d - SEC %d - Node %s - PID %d\n",
            rank, 0, computeNode.name.c_str(), id);
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
            rank, 1, computeNode.name.c_str(), discrete.c_str());
        tmpstr += buffer;
        //PERFSTUBS_METADATA("CPUs allowed", discrete.c_str());
        return tmpstr;
    }
    std::string logThreads(bool shutdown = false) {
        std::string tmpstr;
        static size_t nthreads{0};
        if (shutdown) {
            tmpstr += "\n\t *** Final thread summary: *** \n\n";
            nthreads = 0;
        }
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
};

} // namespace software

} // namespace zerosum