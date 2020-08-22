//
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//
#include <zmq.hpp>
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

enum Status { OK, OFF, WARNING, ERROR, CRITICAL};
const boost::unordered_map<Status,const char*> StatusToString = boost::assign::map_list_of
        (OK, "OK")
        (OFF, "OFF")
        (WARNING, "WARNING")
        (ERROR, "ERROR")
        (CRITICAL, "CRITICAL");

Status random_status() {
    return static_cast<Status>(rand() % 5);
}

std::string generate_json_string(std::string timestamp, bool subsystems_enabled) {
    boost::property_tree::ptree pt;
    pt.put("timestamp", "AliveAt"+timestamp);
    if (subsystems_enabled) {
        pt.put("statuses.subsystem1", StatusToString.at(random_status()));
        pt.put("statuses.subsystem2", StatusToString.at(random_status()));
    } else {
        pt.put("statuses.subsystem1", "false");
        pt.put("statuses.subsystem2", "false");
    }
    std::ostringstream buf;
    boost::property_tree::write_json (buf, pt, false);
    std::string json = buf.str();
    return json;
}

std::string generate_reply(std::string message, bool subsystems_enabled) {
    std::string timestamp = message.substr(12, 10);
    return generate_json_string(timestamp, subsystems_enabled);
}

int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);

    zmq::socket_t main_socket (context, ZMQ_REP);
    main_socket.bind ("tcp://*:12277");

    zmq::socket_t instructions_socket (context, ZMQ_REP);
    instructions_socket.bind ("tcp://*:12278");

    std::cout << "CLIENT" << std::endl;
    bool subsystems_enabled = true;
    while (true) {
        zmq::message_t instructions_request;
        auto instructions_result = instructions_socket.recv (instructions_request, zmq::recv_flags::dontwait);
        if (instructions_result) {
            subsystems_enabled = false;
        }

        zmq::message_t main_request;
        auto main_result = main_socket.recv (main_request, zmq::recv_flags::dontwait);
        if (main_result) {
            //  Receive request
            std::cout << "Received:"<< std::endl << main_request.to_string() << std::endl;
            std::string reply = generate_reply(main_request.to_string(), subsystems_enabled);

            //  Send reply back to client
            main_socket.send (zmq::buffer(reply), zmq::send_flags::none);
            std::cout << "Send:"<< std::endl << reply << std::endl;
        }
    }
    return 0;
}