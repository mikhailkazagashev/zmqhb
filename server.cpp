#include <zmq.hpp>
#include <string>
#include <iostream>
#include <time.h>
#include <thread>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include "signs.h"

uint64_t get_timestamp()
{
    using namespace std::chrono;
    uint64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return timestamp;
}

void heartbeat(zmq::context_t *context, EC_KEY *ec_private_key)
{
    #define HEARTBEAT_INTERVAL  1000000

    zmq::socket_t socket (*context, ZMQ_REQ);
    socket.connect ("tcp://localhost:12277");

    //  Send out heartbeats at regular intervals
    uint64_t heartbeat_at = get_timestamp() + HEARTBEAT_INTERVAL;

    while (true) {
        uint64_t now = get_timestamp ();
        if (now > heartbeat_at) {
            // Create query
            heartbeat_at = now + HEARTBEAT_INTERVAL;
            std::string query = "areYouAlive+"+std::to_string(std::time(nullptr));

            // Create signature
            unsigned char *sig_p = nullptr;
            long sig_size = get_signature(query, ec_private_key, &sig_p);

            //  Send query
            socket.send (zmq::buffer(query), zmq::send_flags::sndmore);
            std::cout << "Send:" << std::endl << query << std::endl;

            //  Send signature
            socket.send ( zmq::buffer(sig_p, sig_size), zmq::send_flags::none);
            std::cout << "Send signature" << std::endl;

            //  Get the reply
            zmq::message_t reply;
            auto result = socket.recv (reply, zmq::recv_flags::none);
            std::cout << "Received:" << std::endl << reply.to_string() << std::endl;
        }
    }
}

int main ()
{
    FILE *f = fopen("private.ec.key", "r");
    if(!f) {
        std::perror("private.ec.key file opening failed");
        return 0;
    }
    EC_KEY *ec_private_key = PEM_read_ECPrivateKey(f, nullptr, nullptr, nullptr);
    fclose(f);

    std::cout << "SERVER" << std::endl;
    std::cout << "Press Enter to send stop instruction" << std::endl;

    zmq::context_t context (1);

    zmq::socket_t instructions_socket (context, ZMQ_REQ);
    instructions_socket.connect ("tcp://localhost:12278");

    // Run heartbeat thread
    std::thread heartbeatThread (heartbeat, &context, ec_private_key);

    // Wait for Enter
    std::cin.get();

    // Send stop
    std::string stop_query = "stop";
    unsigned char *sig_p = nullptr;
    long sig_size = get_signature(stop_query, ec_private_key, &sig_p);
    instructions_socket.send (zmq::buffer(stop_query), zmq::send_flags::sndmore);
    instructions_socket.send ( zmq::buffer(sig_p, sig_size), zmq::send_flags::none);
    std::cout << "Send stop"<< std::endl;

    heartbeatThread.join();
    return 0;
}