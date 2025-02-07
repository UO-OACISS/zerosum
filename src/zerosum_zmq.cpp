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

namespace zerosum {

const char * ipc_location{"ipc:///tmp/zs.feeds.0"};

int ZeroSum::writeToLocalAggregator(const std::string& data) {
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    socket.connect("tcp://localhost:5555");
    //socket.connect(ipc_location);

    // send the request message
    socket.send(zmq::buffer(data), zmq::send_flags::none);

    // wait for reply from server
    zmq::message_t reply{};
    zmq::recv_result_t ret(socket.recv(reply, zmq::recv_flags::none));
    if (ret.has_value() && (EAGAIN == ret.value()))
    {
        // msocket had nothing to read and recv() timed out
        //std::cout << "Not Received " << reply.to_string() << std::endl;
        return 1;
    }

    //std::cout << "Received " << reply.to_string();
    std::cout << std::endl;
    return 0;
}

}
