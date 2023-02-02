#ifndef _KEYS_H_
#define _KEYS_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "./PicoSHA2/picosha2.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "./qTESLA/api.h"
#ifdef __cplusplus
}
#endif

std::vector<std::vector<unsigned char>> GetPublicKeyList();

std::string CharArrayToHexStr(unsigned char* str_in, unsigned long long len);
std::vector<unsigned char> HexStrToCharArray(std::string hex_string);

void write_public_key(std::string filepath, unsigned char* uchar, unsigned long long len);

void write_secret_key(std::string filepath, unsigned char* uchar, unsigned long long len);

int load_secret_key(std::string filepath, unsigned char* uchar, unsigned long long len);



#endif