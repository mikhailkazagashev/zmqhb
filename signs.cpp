#include <openssl/sha.h>
#include <iostream>
#include <openssl/ecdsa.h>
#include <openssl/bio.h>
#include <string.h>
#include <zmq.hpp>
#include "signs.h"

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

long get_signature(std::string input, EC_KEY* ec_private_key, unsigned char **sig_p) {
    // Create hash
    unsigned char hash[SHA256_DIGEST_LENGTH]; // 32 bytes
    if(!simpleSHA256(input, hash))
    {
        std::cout << "Creating hash failed"<< std::endl;
        std::terminate();
    }
    // Create signature
    ECDSA_SIG *signature = ECDSA_do_sign(hash, SHA256_DIGEST_LENGTH, ec_private_key);
    long sig_size = i2d_ECDSA_SIG(signature, sig_p);
    return sig_size;
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