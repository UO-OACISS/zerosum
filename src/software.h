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

#pragma once

#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <array>
#include "utils.h"

namespace zerosum {

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
        std::map<std::string, std::string> _fields,
        std::vector<uint32_t> allowed_hwt) : id(_id), rank(_rank), size(_size) {
        for (auto t : allowed_hwt) {
            hwthreads.insert(t);
        }
        // pid should be the same as tid, but just in case...
        threads.insert(std::pair(id, LWP(gettid(), allowed_hwt, _fields, ThreadType::Main)));
        if (_fields.count("executable") > 0) {
            executable = _fields["executable"];
        }
        fields = _fields;
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
    std::string executable;
    std::map<std::string, std::string> fields;
    std::map<int, std::pair<size_t, size_t>> sentBytes;
    std::map<int, std::pair<size_t, size_t>> recvBytes;

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
#ifdef USE_MPI
        tmpstr += "\nP2P Communication Summary:\n";
        for (auto b : sentBytes) {
            auto dest = b.first;
            size_t count = b.second.first;
            size_t bytes = b.second.second;
            tmpstr += "Sent " + std::to_string(bytes)
                    + " bytes to rank " + std::to_string(dest)
                    + " in " + std::to_string(count) + " calls\n";
        }
        for (auto b : recvBytes) {
            auto source = b.first;
            size_t count = b.second.first;
            size_t bytes = b.second.second;
            tmpstr += "Received " + std::to_string(bytes)
                    + " bytes from rank " + std::to_string(source)
                    + " in " + std::to_string(count) + " calls\n";
        }
        tmpstr += "\n";
#endif
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
    std::string getSummary(bool details = true) {
        std::string tmpstr;
        if (details) {
            tmpstr += "\nProcess Summary:\n";
        }
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
        if (details) {
            snprintf(buffer, 1024,
                "MPI %03d - PID %d - Node %s - CPUs allowed: [%s]\n",
                rank, id, computeNode->name.c_str(), discrete.c_str());
        } else {
            snprintf(buffer, 1024,
                "PID %d - %s - CPUs allowed: [%s]\n",
                id, executable.c_str(), discrete.c_str());
        }
        tmpstr += buffer;

        if (details) {
            // print total threads
            tmpstr += "\nLWP (thread) Summary:\n";
            for (auto t : threads) {
                tmpstr += t.second.getSummary();
                tmpstr += "\n";
            }
            // print assigned cores
            tmpstr += computeNode->getSummary(hwthreads);
        }

#ifdef USE_MPI
        tmpstr += "\nP2P Communication Summary:\n";
        for (auto b : sentBytes) {
            auto dest = b.first;
            size_t count = b.second.first;
            size_t bytes = b.second.second;
            tmpstr += "Sent " + std::to_string(bytes)
                    + " bytes to rank " + std::to_string(dest)
                    + " in " + std::to_string(count) + " calls\n";
        }
        for (auto b : recvBytes) {
            auto source = b.first;
            size_t count = b.second.first;
            size_t bytes = b.second.second;
            tmpstr += "Received " + std::to_string(bytes)
                    + " bytes from rank " + std::to_string(source)
                    + " in " + std::to_string(count) + " calls\n";
        }
#endif
        return tmpstr;
    }

    void recordSentBytes(int rank, size_t bytes) {
        if (sentBytes.count(rank) == 0) {
            sentBytes.insert(std::pair(rank, std::pair(0, 0)));
        }
        auto& tmp = sentBytes[rank];
        tmp.first++;
        tmp.second += bytes;
    }

    void recordRecvBytes(int rank, size_t bytes) {
        if (recvBytes.count(rank) == 0) {
            recvBytes.insert(std::pair(rank, std::pair(0, 0)));
        }
        auto& tmp = recvBytes[rank];
        tmp.first++;
        tmp.second += bytes;
    }
};

} // namespace software

} // namespace zerosum