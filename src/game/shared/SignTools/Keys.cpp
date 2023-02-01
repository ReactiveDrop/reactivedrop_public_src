#include "Keys.h"

std::string char_array_to_hex_string(unsigned char* str_in, unsigned long long len)
{
    std::stringstream ss;
    for (int i = 0; i < len; i++)
    {
        ss <<  std::setfill('0') << std::setw(sizeof(char) * 2)
            << std::hex << (int)(str_in[i]);
    }
    std::string str = ss.str();
    std::cout << "Signed message:"  << std::endl << str << std::endl << std::endl;
    return str;
}

std::vector<unsigned char> hex_string_to_char_array(std::string hex_string)
{
    std::string temp;
    for (unsigned int i = 0; i < hex_string.length(); i += 2)
    {
        temp.push_back(hex_string[i]);
        temp.push_back(hex_string[i + 1]);
        temp.push_back(' ');
    }
    std::istringstream hex_chars_stream(temp);
    std::vector<unsigned char> bytes;
    unsigned int c;
    while (hex_chars_stream >> std::hex >> c)
    {
        bytes.push_back(c);
    }

    return bytes;
}

void write_public_key(std::string filepath, unsigned char* uchar, unsigned long long len)
{
    std::ofstream ofs;
    ofs.open(filepath + "pubkic_key.txt");
    std::stringstream ss;
    int* p = (int*)uchar;
    unsigned long long int count = len / 4;
    for (int i = 0; i < count; i++)
    {
        ss << "0x"
            << std::setfill('0') << std::setw(sizeof(int) * 2)
            << std::hex << p[i] << ",";
    }
    std::string str = ss.str();
    str.erase(str.length() - 1);
    ofs << "unsigned int public_key_int[" << count << "] = {" << std::endl << str << std::endl << "};";
    ofs.close();
    std::cout << "Public key is successfully written. You can now submit public_key.txt to game devs." << std::endl << std::endl;
    return;
}

void write_secret_key(std::string filepath, unsigned char* uchar, unsigned long long len)
{
    std::ofstream ofs;
    ofs.open(filepath + "secret_key.bin", std::ios::binary);
    ofs.write((char*)uchar, len);
    ofs.close();
    std::cout <<"Secret key is successfully written. Please carefully keep your secret_key.bin. DO NOT send it to anyone else!!!" << std::endl << std::endl;
    return;
}

int load_secret_key(std::string filepath, unsigned char* uchar, unsigned long long len)
{
    std::ifstream ifs;
    std::cout << "Loading secret key from file:" << filepath + "secret_key.bin" << std::endl;
    ifs.open(filepath + "secret_key.bin", std::ios::binary);
    if (!ifs.is_open())
    {
        std::cout << "Unable to load secret key!" << std::endl;
        return 0;
    }
    ifs.read((char*)uchar, len);
    ifs.close();
    std::cout << "Successfully load secret key!" << std::endl;
    return 1;
}


