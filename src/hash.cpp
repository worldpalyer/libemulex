//
//  hash.cpp
//  libemulex
//
//  Created by Centny on 3/23/17.
//
//

#include "emulex/hash.hpp"
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace emulex {

file_hash read_file_hash(std::string filepath, bool bmd4, bool bmd5, bool bsha1) {
    std::fstream file(filepath, std::fstream::in);
    if (!file) {
        throw std::exception();
    }
    try {
        file_hash hash = read_file_hash(&file, bmd4, bmd5, bsha1);
        file.close();
        return hash;
    } catch (const std::exception& e) {
        file.close();
        throw e;
    }
}

std::string hash_tos(const char* hash, size_t size) {
    char buf[2 * size + 1];
    for (size_t i = 0; i < size; i++) {
        sprintf(buf + i * 2, "%02x", (unsigned char)hash[i]);
    }
    buf[2 * size] = 0;
    return std::string(buf);
}

file_hash read_file_hash(std::fstream* file, bool bmd4, bool bmd5, bool bsha1) {
    file_hash hash;
    MD4_CTX fmd4;
    if (bmd4) {
        MD4_Init(&fmd4);
    }
    MD5_CTX fmd5;
    if (bmd5) {
        MD5_Init(&fmd5);
    }
    SHA_CTX fsha;
    if (bsha1) {
        SHA1_Init(&fsha);
    }
    hash.size = 0;
    file->seekg(0);
    file->seekp(0);
    char buf[9728];
    size_t readed = 0;
    while (!file->eof()) {
        MD4_CTX pmd4;
        if (bmd4) {
            MD4_Init(&pmd4);
        }
        for (int i = 0; i < 1000 && !file->eof(); i++) {
            file->read(buf, 9728);
            readed = file->gcount();
            hash.size += readed;
            if (bmd4) {
                MD4_Update(&pmd4, buf, readed);
            }
            if (bmd5) {
                MD5_Update(&fmd5, buf, readed);
            }
            if (bsha1) {
                SHA1_Update(&fsha, buf, readed);
            }
        }
        if (bmd4) {
            unsigned char digest[MD4_DIGEST_LENGTH];
            MD4_Final(digest, &pmd4);
            hash.parts.push_back(hash_tos((const char*)digest, MD4_DIGEST_LENGTH));
            MD4_Update(&fmd4, digest, MD4_DIGEST_LENGTH);
        }
    }
    if (bmd4) {
        unsigned char digest[MD4_DIGEST_LENGTH];
        MD4_Final(digest, &fmd4);
        if (hash.parts.size() == 1) {
            hash.md4 = hash.parts[0];
        } else {
            hash.md4 = hash_tos((const char*)digest, MD4_DIGEST_LENGTH);
        }
    }
    if (bmd5) {
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5_Final(digest, &fmd5);
        hash.md5 = hash_tos((const char*)digest, MD5_DIGEST_LENGTH);
    }
    if (bsha1) {
        unsigned char digest[SHA_DIGEST_LENGTH];
        SHA1_Final(digest, &fsha);
        hash.sha1 = hash_tos((const char*)digest, SHA_DIGEST_LENGTH);
    }
    return hash;
}
}
