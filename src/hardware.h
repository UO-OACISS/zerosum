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
#include <string>
#include <iostream>
#include "utils.h"
#include "perfstubs.h"
#include "zerosum.h"

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
        id = stoi(props["RT_GPU_ID"]);
        timerPrefix = std::string("GPU: Device ") + props["RT_GPU_ID"] + " ";
    }
    GPU() = default;
    ~GPU() = default;
    uint32_t id;
    std::string timerPrefix;
    std::map<std::string, std::string> properties;
    std::map<std::string, std::vector<std::string>> stat_fields;
    void updateFields(std::map<std::string, std::string> fields) {
        for (auto f : fields) {
            if (stat_fields.count(f.first) == 0) {
                std::vector<std::string> v;
                stat_fields.insert(std::pair(f.first, v));
            }
            stat_fields[f.first].push_back(f.second);
            /* CAn't do this here - the PerfStubs API expects a static object
             * and we are reusing this as a generic call - not possible. */
            //std::string tmpstr{timerPrefix + f.first};
            //PERFSTUBS_SAMPLE_COUNTER(tmpstr.c_str(), stof(f.second));
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
            double _min = stod(sf.second[0]);
            double _max = stod(sf.second[0]);
            if (sf.first.compare("Energy Average (J)") == 0 ||
                sf.first.compare("GFX Activity %") == 0 ||
                sf.first.compare("Memory Activity %") == 0) {
                auto previous = sf.second[0];
                _min = 0.0;
                _max = 0.0;
                for (auto v : sf.second) {
                    auto tmp = strSub(v,previous,total);
                    previous = v;
                    _max = std::max(_max,stod(tmp));
                    _min = std::min(_min,stod(tmp));
                }
                double average = total/(double)(std::max(size_t(1),sf.second.size()-1));
                tmpstr += std::to_string(_min) + " " +
                          std::to_string(average) + " " +
                          std::to_string(_max);
            } else {
                for (auto v : sf.second) {
                    total += atof(v.c_str());
                    _max = std::max(_max,stod(v));
                    _min = std::min(_min,stod(v));
                }
                double average = total/(double)(std::max(size_t(1),sf.second.size()));
                tmpstr += std::to_string(_min) + " " +
                          std::to_string(average) + " " +
                          std::to_string(_max);
            }
            tmpstr += "\n";
        }
        return tmpstr;
    }
    std::string reportMemory(void) {
        std::string tmpstr;
        for (auto sf : stat_fields) {
            if (sf.first.compare("Total VRAM Bytes") == 0) {
                tmpstr += "GPU ";
                tmpstr += sf.first;
                tmpstr += ": ";
                tmpstr += sf.second.back();
                tmpstr += ", ";
            }
            if (sf.first.compare("Used VRAM Bytes") == 0) {
                tmpstr += "GPU ";
                tmpstr += sf.first;
                tmpstr += ": ";
                tmpstr += sf.second.back();
            }
        }
        return tmpstr;
    }
};

class ComputeNode {
public:
    ComputeNode(std::string _name, bool details = false) :
        name(_name), ncpus(std::thread::hardware_concurrency()) {
        hwThreads.reserve(ncpus);
        for (unsigned i = 0 ; i < ncpus ; i++) {
            hwThreads.push_back(HWT(i));
        }
        doDetails = details;
    }
    ComputeNode() = default;
    ~ComputeNode() = default;
    std::string name;
    unsigned ncpus;
    std::vector<HWT> hwThreads;
    std::vector<GPU> gpus;
    bool doDetails;
    std::map<std::string, std::vector<std::string>> stat_fields;
    /* Update the node-level properties */
    void updateFields(std::map<std::string, std::string> fields) {
        for (auto f : fields) {
            if (stat_fields.count(f.first) == 0) {
                std::vector<std::string> v;
                stat_fields.insert(std::pair(f.first, v));
            }
            stat_fields[f.first].push_back(f.second);
        }
    }
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
    /* Update the hwthread-level properties */
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
            outstr += "GPU " + gpu.properties["RT_GPU_ID"] + " - (metric: min  avg  max)\n";
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
            outstr += "GPU " + gpu.properties["RT_GPU_ID"] + " - (metric: min  avg  max)\n";
            outstr += gpu.getSummary();
            outstr += "\n";
        }
        if (doDetails) {
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
        }
        return outstr;
    }
    std::string reportMemory(void) {
        std::string tmpstr;
        for (auto sf : stat_fields) {
            if (sf.first.compare("MemTotal") == 0 ||
                sf.first.compare("MemFree") == 0 ||
                sf.first.compare("MemAvailable") == 0) {
                tmpstr += sf.first;
                tmpstr += ": ";
                tmpstr += sf.second.back();
                tmpstr += ", ";
            }
        }
        for (auto gpu : gpus) {
            tmpstr += gpu.reportMemory();
        }
        tmpstr += "\n";
        return tmpstr;
    }
};

} // namespace hardware

} // namespace zerosum