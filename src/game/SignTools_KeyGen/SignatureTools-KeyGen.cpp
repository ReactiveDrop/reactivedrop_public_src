#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "../shared/SignTools/Keys.h"

unsigned char public_key[CRYPTO_PUBLICKEYBYTES];
unsigned char secret_key[CRYPTO_SECRETKEYBYTES];

int main()
{
    std::cout << std::endl 
        << "===================================================================================" << std::endl 
        << "\tGenerating signature key pairs using scheme qTESLA, system " << CRYPTO_ALGNAME << std::endl 
        << "===================================================================================" << std::endl << std::endl;

    std::cout << "CRYPTO_PUBLICKEY_BYTES:" << CRYPTO_PUBLICKEYBYTES << std::endl
        << "CRYPTO_SECRETKEY_BYTES:" << (int)CRYPTO_SECRETKEYBYTES << std::endl
        << "CRYPTO_SIGNATURE_BYTES:" << CRYPTO_BYTES << std::endl << std::endl;

    crypto_sign_keypair(public_key, secret_key);

    std::string file_path = ".\\";

    write_public_key(file_path, public_key, CRYPTO_PUBLICKEYBYTES);
    write_secret_key(file_path, secret_key, CRYPTO_SECRETKEYBYTES);

    std::cout << "Press any key to exit...";
    std::cin.get();
    return 0;
}