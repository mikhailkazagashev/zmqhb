//
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <time.h>

uint64_t get_timestamp()
{
    using namespace std::chrono;
    uint64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return timestamp;
}

int main ()
{
    #define HEARTBEAT_INTERVAL  1000000

    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REQ);
    socket.connect ("tcp://localhost:12277");

    std::cout << "SERVER" << std::endl;

    //  Send out heartbeats at regular intervals
    uint64_t heartbeat_at = get_timestamp() + HEARTBEAT_INTERVAL;

    while (true) {
        uint64_t now = get_timestamp ();
        if (now > heartbeat_at) {
            //  Send request
            heartbeat_at = now + HEARTBEAT_INTERVAL;
            std::string query = "areYouAlive+"+std::to_string(std::time(nullptr));
            socket.send (zmq::buffer(query), zmq::send_flags::none);
            std::cout << "Send:" << std::endl << query << std::endl;

            //  Get the reply
            zmq::message_t reply;
            socket.recv (reply, zmq::recv_flags::none);
            std::cout << "Received:" << std::endl << reply.to_string() << std::endl;
        }
    }
    return 0;
}