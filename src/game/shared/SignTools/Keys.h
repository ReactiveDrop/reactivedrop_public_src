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

std::string CharArrayToHexString(unsigned char* array, unsigned long long len);
std::vector<unsigned char> HexStringToCharArray(std::string hexStr);

void WritePublicKey(std::string filePath, unsigned char* pKey, unsigned long long len);

void WriteSecretKey(std::string filePath, unsigned char* sKey, unsigned long long len);

int LoadSecretKey(std::string filePath, unsigned char* sKey, unsigned long long len);



#endif