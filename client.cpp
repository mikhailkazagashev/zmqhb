#include <zmq.hpp>
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

bool simpleSHA256(std::string input, unsigned char* md)
{
    SHA256_CTX context;
    if(!SHA256_Init(&context))
        return false;

    if(!SHA256_Update(&context, input.c_str(), input.length()))
        return false;

    if(!SHA256_Final(md, &context))
        return false;

    return true;
}

bool check_signature(std::string input, EC_KEY* ec_pub_key, void *sig, long size) {
    unsigned char hash[SHA256_DIGEST_LENGTH]; // 32 bytes
    if(!simpleSHA256(input, hash))
    {
        std::cout << "Creating hash failed"<< std::endl;
        std::terminate();
    }
    const unsigned char *sig_p = NULL;
    sig_p = static_cast<const unsigned char*>(sig);
    auto *signature = d2i_ECDSA_SIG(NULL, &sig_p, size);
    return ECDSA_do_verify(hash, SHA256_DIGEST_LENGTH, signature, ec_pub_key) == 1;
}

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
    FILE * f1 = fopen("public.pem", "r");
    EC_KEY *ec_pub_key = PEM_read_EC_PUBKEY(f1, NULL, NULL, NULL);
    fclose(f1);

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
        auto instructions_result_p2 = instructions_socket.recv (instructions_request_p2, zmq::recv_flags::dontwait);
        if (instructions_result_p1 and instructions_result_p2) {
            if (check_signature(instructions_request_p1.to_string(), ec_pub_key, instructions_request_p2.data(), instructions_request_p2.size())) {
                if (instructions_request_p1.to_string() == "stop") {
                    std::cout << "Stop instruction Received"<< std::endl;
                    subsystems_enabled = false;
                }
            } else {
                std::cout << "Instruction signature check failed" << std::endl;
            }
            instructions_socket.send (zmq::message_t(), zmq::send_flags::none);
        }

        zmq::message_t main_request_p1, main_request_p2;
        std::string reply;
        auto main_result_p1 = main_socket.recv (main_request_p1, zmq::recv_flags::dontwait);
        if (!main_request_p1.more()) {
            reply = "second message expected" ;
        } else {
            auto main_result_p2 = main_socket.recv(main_request_p2, zmq::recv_flags::dontwait);
            if (main_result_p1 and main_result_p2) {
                std::string query = main_request_p1.to_string();
                std::cout << "Received:"<< std::endl << query<< std::endl;

                if (check_signature(query, ec_pub_key, main_request_p2.data(), main_request_p2.size())) {
                    reply = generate_reply(main_request_p1.to_string(), subsystems_enabled);
                } else {
                    reply = "Signature check failed";
                    std::cout << reply << std::endl;
                }
                //  Send reply back to client
                main_socket.send (zmq::buffer(reply), zmq::send_flags::none);
                std::cout << "Send:"<< std::endl << reply << std::endl;
            }
        }
    }
    return 0;
}