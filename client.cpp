#include <zmq.hpp>
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include "signs.h"

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

std::string generate_reply(std::string message, bool subsystems_enabled) {
    std::string timestamp = message.substr(12, 10);
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

int main ()
{
    // Read key
    FILE * f = fopen("public.pem", "r");
    if(!f) {
        std::perror("public.pem file opening failed");
        return 0;
    }

    EC_KEY *ec_pub_key = PEM_read_EC_PUBKEY(f, nullptr, nullptr, nullptr);
    fclose(f);

    std::cout << "CLIENT" << std::endl;

    zmq::context_t context (1);

    zmq::socket_t main_socket (context, ZMQ_REP);
    main_socket.bind ("tcp://*:12277");

    zmq::socket_t instructions_socket (context, ZMQ_REP);
    instructions_socket.bind ("tcp://*:12278");

    bool subsystems_enabled = true;
    while (true) {
        zmq::message_t instructions_request_p1, instructions_request_p2;
        auto instructions_result_p1 = instructions_socket.recv (instructions_request_p1, zmq::recv_flags::dontwait);
        std::string instructions_reply;
        if (instructions_result_p1) {
            //TODO: add try catch
            if (!instructions_request_p1.more()) {
                instructions_reply = "Second message with signature missed" ;}
            else {
                auto instructions_result_p2 = instructions_socket.recv (instructions_request_p2, zmq::recv_flags::dontwait);
                if (instructions_result_p2) {
                    if (check_signature(instructions_request_p1.to_string(), ec_pub_key, instructions_request_p2.data(), instructions_request_p2.size())) {
                        if (instructions_request_p1.to_string() == "stop") {
                            instructions_reply = "OK";
                            subsystems_enabled = false;
                        }
                    } else {
                        instructions_reply = "Instruction signature check failed";
                    }
                }
            }
            std::cout << "Send:"<< std::endl << instructions_reply << std::endl;
            instructions_socket.send (zmq::buffer(instructions_reply), zmq::send_flags::none);
        }

        zmq::message_t main_request_p1, main_request_p2;
        std::string reply;
        auto main_result_p1 = main_socket.recv (main_request_p1, zmq::recv_flags::dontwait);
        if (main_result_p1) {
            //TODO: add try catch
            if (!main_request_p1.more()) {
                reply = "Second message with signature missed" ;
            } else {
                auto main_result_p2 = main_socket.recv(main_request_p2, zmq::recv_flags::dontwait);
                if (main_result_p2) {
                    std::string query = main_request_p1.to_string();
                    std::cout << "Received:"<< std::endl << query<< std::endl;

                    if (check_signature(query, ec_pub_key, main_request_p2.data(), main_request_p2.size())) {
                        reply = generate_reply(main_request_p1.to_string(), subsystems_enabled);
                    } else {
                        reply = "Signature check failed";
                        std::cout << reply << std::endl;
                    }
                }
            }
            //  Send reply back to client
            main_socket.send (zmq::buffer(reply), zmq::send_flags::none);
            std::cout << "Send:"<< std::endl << reply << std::endl;
        }
    }
    return 0;
}