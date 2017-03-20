//
//  encoding.hpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#ifndef enc_encoding_hpp
#define enc_encoding_hpp
#include <stdio.h>
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include "fail.hpp"
#include "data.hpp"

namespace emulex {
namespace db {
using namespace boost;
const uint8_t STR_TAG = 0x2;
const uint8_t INT_TAG = 0x3;
const uint8_t FLOAT_TAG = 0x4;
/*
 the ed2k base encoding for int/string/tag
 */
class Encoding {
   public:
    Encoding();
    virtual void reset();
    virtual size_t size();
    boost::asio::streambuf &buf();
    const char *cbuf();
    virtual Data encode();
    Encoding &put(Data &data);
    Encoding &put(const char *val, size_t len);
    Encoding &put(uint8_t val, bool big = false);
    Encoding &put(uint16_t val, bool big = false);
    Encoding &put(uint32_t val, bool big = false);
    Encoding &put(uint64_t val, bool big = false);
    //    Encoding& put(float val);
    Encoding &putv(const char *name, const char *val);
    Encoding &putv(Data &name, Data &val);
    Encoding &putv(uint8_t name, const char *val);
    Encoding &putv(uint8_t name, Data &val);
    Encoding &putv(const char *name, Data &val);
    Encoding &putv(const char *name, uint32_t val);
    Encoding &putv(uint8_t name, uint32_t val);
    Encoding &putv(const char *name, uint16_t val);
    Encoding &putv(uint8_t name, uint16_t val);
    //  Encoding &put(const char *name, float val);
    //  Encoding &put(uint8_t name, float val);
    Encoding &putv(const char *name, uint64_t val);
    Encoding &putv(uint8_t name, uint64_t val);
    virtual void print(char *buf = 0);
    Data deflate();
    //    Data lzmadef();

   protected:
    boost::asio::streambuf buf_;
};

/*
 the ed2k login frame.
 */
class Decoding {
   public:
    Decoding(Data &data);
    virtual ~Decoding();
    template <typename T, std::size_t n_bits>
    T get(bool big = false) {
        if (data->len - offset < n_bits) {
            throw Fail("decode fail with not enough data, expect %ld, but %ld", n_bits, data->len - offset);
        }
        T val;
        if (big) {
            val = boost::endian::detail::load_big_endian<T, n_bits>(data->data + offset);
        } else {
            val = boost::endian::detail::load_little_endian<T, n_bits>(data->data + offset);
        }
        offset += n_bits;
        return val;
    }
    void get(char *name, size_t len);
    void get(char *name, uint16_t &nlen);
    void get(char *name, uint16_t &nlen, char *value, uint16_t &vlen);
    void get(char *name, uint16_t &nlen, uint32_t &vlen);
    Data getstring(size_t len = 0);
    Data getdata(size_t len, bool iss = false);

    void inflate();

   public:
    Data data;
    size_t offset;
};
typedef std::pair<uint32_t, uint16_t> Address;
std::string addr_cs(Address &addr);
//////////end encoding//////////
}
}

#endif /* enc_encoding_hpp */
