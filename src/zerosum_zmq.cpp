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

#include "zerosum.h"
#include <cstdio>
#include <regex>
#include <iostream>
#include <vector>
#include <string>
#include <zmq.hpp>
#include <sys/time.h>

namespace zerosum {

const char * ipc_location{"ipc:///tmp/zmq_test"};

int ZeroSum::writeToLocalAggregator(const std::string& data) {
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};
    static std::string bindPoint{parseString("ZS_AGGREGATION_BINDPOINT", ipc_location)};
    int immediate = 1;
    int send_timeout = 0;
    int recv_timeout = -1; // infinite, the default

    struct timeval stamp;

    try {
        // construct a REQ (request) socket and connect to interface
        zmq::socket_t socket{context, ZMQ_PUSH}; // changed to PUSH
        socket.setsockopt(ZMQ_IMMEDIATE, &immediate, sizeof(immediate));
        //socket.setsockopt(ZMQ_SNDTIMEO, &send_timeout, sizeof(send_timeout));
        //socket.setsockopt(ZMQ_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
        // make the connection
        // std::cout << "Connecting to " + bindPoint + "..." << std::endl;
        socket.connect(bindPoint);
        {
            // get time of message sent
            gettimeofday(&stamp, NULL);

            double timestamp = stamp.tv_sec + (stamp.tv_usec / 1000000.0);

            std::string time_string = "time: " + std::to_string(timestamp) + ",";

            std::string time_and_data = time_string + data;
            // send the request message
            // std::cout << "Sending..." << std::endl;
            zmq::send_result_t ret(socket.send(zmq::buffer(time_and_data), zmq::send_flags::none));

            // got rid of all recv stuff
            if (ret.has_value() && (EAGAIN == ret.value()))
            {
                // send failed
                std::cerr << "Send failed" << std::endl;
                return 1;
            }
        }
    } catch (const zmq::error_t& e) {
        //std::cerr << "Exception" << std::endl;
        return 1; // Indicate failure
    }
    return 0;
}

}
