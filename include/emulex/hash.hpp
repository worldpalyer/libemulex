//
//  hash.hpp
//  libemulex
//
//  Created by Centny on 3/23/17.
//
//

#ifndef hash_h
#define hash_h
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <string>

namespace emulex {
struct file_hash {
    uint64_t size;
    std::string md4;
    std::vector<std::string> parts;
    std::string md5;
    std::string sha1;
};

std::string hash_tos(const char* hash, size_t size);
file_hash read_file_hash(std::string filepath, bool bmd4 = false, bool bmd5 = false, bool bsha1 = false);
file_hash read_file_hash(std::fstream* file, bool bmd4 = false, bool bmd5 = false, bool bsha1 = false);
}

#endif /* hash_h */
