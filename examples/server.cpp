#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#include <zmq.hpp>

int main()
{
    using namespace std::chrono_literals;

    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket{context, zmq::socket_type::rep};
    socket.bind("tcp://*:5555");
    //socket.bind("ipc:///tmp/zs.feeds.0");

    // prepare some static data for responses
    const std::string data{"success"};

    for (;;)
    {
        zmq::message_t request;

        // receive a request from client
        socket.recv(request, zmq::recv_flags::none);
        std::cout << "Received " << request.to_string() << std::endl;

        // send the reply to the client
        socket.send(zmq::buffer(data), zmq::send_flags::none);
    }

    return 0;
}
