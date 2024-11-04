/*
   Utility to extract MPI P2P data from zerosum logs and
   represent them as CSV files that a post-processing script
   can utilize.
   */
/*
sentfile="sent.heatmap.csv"
rm -f $sentfile

for i in {000..512} ; do
    echo ${i}
    file="zs.${i}.log"
    #for line in `grep "Sent " ${i}` ; do
    grep "Sent " ${file} | while read -r line ; do
        bytes=`echo ${line} | awk '{ print $2 }'`
        rank=`echo ${line} | awk '{ print $6 }'`
        calls=`echo ${line} | awk '{ print $8 }'`
        echo $i,$rank,$bytes,$calls > $sentfile
    done
done
}
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <algorithm>
#include <vector>
#include <cstdint>

void write_vector(
    const std::vector<std::vector<size_t>>& data,
    const std::string filename, uint32_t nranks) {
    std::ofstream output(filename);
    if (!output) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    for (uint32_t i = 0 ; i < nranks ; i++) {
        for (uint32_t j = 0 ; j < nranks ; j++) {
            output << data[i][j];
            if (j < nranks-1) { output << ','; }
        }
        output << std::endl;
    }
    output.close();
}

int main(int argc, char * argv[]) {
    std::vector<std::vector<size_t>> sent_count;
    std::vector<std::vector<size_t>> sent_bytes;
    std::vector<std::vector<size_t>> recv_count;
    std::vector<std::vector<size_t>> recv_bytes;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <nranks>" << std::endl;
    }
    uint32_t nranks = atoi(argv[1]);

    for (uint32_t i = 0; i < nranks; i++) {
        std::vector<size_t> tmp;
        for (uint32_t j = 0; j < nranks; j++) {
            tmp.push_back(0);
        }
        sent_count.push_back(tmp);
        recv_count.push_back(tmp);
        sent_bytes.push_back(tmp);
        recv_bytes.push_back(tmp);
    }

    size_t n = 3;
    for (uint32_t i = 0; i < nranks; i++) {
        std::string instr = std::to_string(i);
        int precision = n - std::min(n,instr.size());
        std::string padded = std::string(precision, '0').append(instr);
        std::string file = "zs." + padded + ".log";
        std::ifstream input(file);

        if (!input) {
            std::cerr << "Error opening file: " << file << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(input, line)) {
            if (line.find("Sent ") != std::string::npos) {
                std::istringstream iss(line);
                std::string keyword, bytes, rank, calls;
                iss >> keyword >> bytes >> keyword >> keyword >> keyword >> rank >> keyword >> calls;
                sent_count[i][std::stol(rank)] = std::stol(calls);
                sent_bytes[i][std::stol(rank)] = std::stol(bytes);
            }
            if (line.find("Received ") != std::string::npos) {
                std::istringstream iss(line);
                std::string keyword, bytes, rank, calls;
                iss >> keyword >> bytes >> keyword >> keyword >> keyword >> rank >> keyword >> calls;
                recv_count[i][std::stol(rank)] = std::stol(calls);
                recv_bytes[i][std::stol(rank)] = std::stol(bytes);
            }
        }

        input.close();
    }

    /* write out the NxN csv files for simplicity in post-processing */

    write_vector(sent_count, std::string("sent.count.nxn.heatmap.csv"), nranks);
    write_vector(sent_bytes, std::string("sent.bytes.nxn.heatmap.csv"), nranks);
    write_vector(recv_count, std::string("recv.count.nxn.heatmap.csv"), nranks);
    write_vector(recv_bytes, std::string("recv.bytes.nxn.heatmap.csv"), nranks);
    return 0;
}

