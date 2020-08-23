#include <openssl/sha.h>
#include <iostream>
#include <openssl/ecdsa.h>
#include <openssl/bio.h>
#include <string.h>
#include <zmq.hpp>

bool simpleSHA256(std::string input, unsigned char* md);
long get_signature(std::string input, EC_KEY* ec_private_key, unsigned char **sig_p);
bool check_signature(std::string input, EC_KEY* ec_pub_key, void *sig, long size);