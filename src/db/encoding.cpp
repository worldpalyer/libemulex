//
//  encoding.cpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#include "../../include/emulex/db/encoding.hpp"
#include <zlib.h>
namespace emulex {
namespace db {

Encoding::Encoding() {}

void Encoding::reset() { buf_.consume(buf_.size()); }

// const char *Encoding::Encoding::data() { return asio::buffer_cast<const char *>(buf_.data()); }

size_t Encoding::Encoding::size() { return buf_.size(); }
boost::asio::streambuf &Encoding::buf() { return buf_; }
const char *Encoding::cbuf() { return boost::asio::buffer_cast<const char *>(buf_.data()); }
Data Encoding::encode() { return BuildData(boost::asio::buffer_cast<const char *>(buf_.data()), buf_.size()); }

Encoding &Encoding::put(Data &data) {
    put(data->data, data->len);
    return *this;
}

Encoding &Encoding::put(const char *val, size_t len) {
    buf_.sputn(val, len);
    return *this;
}
Encoding &Encoding::put(uint8_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint8_t, 8>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint8_t, 8>(val).data();
    }
    buf_.sputn(data, 1);
    return *this;
}
Encoding &Encoding::put(uint16_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint16_t, 16>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint16_t, 16>(val).data();
    }
    buf_.sputn(data, 2);
    return *this;
}
Encoding &Encoding::put(uint32_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint32_t, 32>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint32_t, 32>(val).data();
    }
    buf_.sputn(data, 4);
    return *this;
}
Encoding &Encoding::put(uint64_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint64_t, 64>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint64_t, 64>(val).data();
    }
    buf_.sputn(data, 8);
    return *this;
}
//    Encoding& Encoding::put(float val){
//      LittleEndianBuf::put<uint32_t, 32>(val);
//      return *this;
//    }
Encoding &Encoding::putv(const char *name, const char *val) {
    put(STR_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen);
    uint16_t vlen = strlen(val);
    put(vlen), put(val, vlen);
    return *this;
}
Encoding &Encoding::putv(Data &name, Data &val) {
    put(STR_TAG);
    put((uint16_t)name->len), put(name->data, name->len);
    put((uint16_t)val->len), put(val->data, val->len);
    return *this;
}
Encoding &Encoding::putv(uint8_t name, const char *val) {
    put(STR_TAG);
    put((uint16_t)1), put(name);
    uint16_t vlen = strlen(val);
    put(vlen), put(val, vlen);
    return *this;
}
Encoding &Encoding::putv(const char *name, Data &val) {
    put(STR_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen);
    put((uint16_t)val->len), put(val->data, val->len);
    return *this;
}
Encoding &Encoding::putv(uint8_t name, Data &val) {
    put(STR_TAG);
    put((uint16_t)1), put(name);
    put((uint16_t)val->len), put(val->data, val->len);
    return *this;
}
Encoding &Encoding::putv(const char *name, uint16_t val) {
    put(INT_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen), put(val);
    return *this;
}
Encoding &Encoding::putv(uint8_t name, uint16_t val) {
    put(INT_TAG);
    put((uint16_t)1), put(name), put(val);
    return *this;
}
Encoding &Encoding::putv(const char *name, uint32_t val) {
    put(INT_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen), put(val);
    return *this;
}
Encoding &Encoding::putv(uint8_t name, uint32_t val) {
    put(INT_TAG);
    put((uint16_t)1), put(name), put(val);
    return *this;
}
//    Encoding& Encoding::put(const char* name, float val){
//      put(FLOAT_TAG);
//      put((uint16_t)strlen(name));
//      put(name, strlen(name));
//      put((uint16_t)4);
//      put(val);
//    }
//    Encoding& Encoding::put(uint8_t name, float val){
//      put(FLOAT_TAG);
//      put((uint16_t)1);
//      put(name);
//      put((uint16_t)4);
//      put(val);
//    }

Encoding &Encoding::putv(const char *name, uint64_t val) {
    put(INT_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen), put(val);
    return *this;
}
Encoding &Encoding::putv(uint8_t name, uint64_t val) {
    put(INT_TAG);
    put((uint16_t)1), put(name), put(val);
    return *this;
}

void Encoding::print(char *buf) {
    Data data = encode();
    size_t len = data->len;
    char tbuf_[102400];
    char *tbuf = buf;
    if (tbuf == 0) {
        tbuf = tbuf_;
    }
    size_t tlen = sprintf(tbuf, "ENC(%ld):", len);
    for (size_t i = 0; i < len; i++) {
        tlen += sprintf(tbuf + tlen, "%2x ", (uint8_t)data->data[i]);
    }
    tbuf[tlen] = 0;
    if (buf == 0) {
        printf("%s\n", tbuf);
    }
}

Data Encoding::deflate() {
    auto data = encode();
    data->deflate();
    return data;
}

//    Data Encoding::lzmadef(){
//        auto data=encode();
//        data->lzmadef();
//        return data;
//    }

Decoding::Decoding(Data &data) {
    this->data = data;
    this->offset = 0;
}
Decoding::~Decoding() {}
void Decoding::get(char *buf, size_t len) {
    if (data->len < offset + len) {
        throw Fail("decode fail with not enough data, expect %ld, but %ld", len, data->len - offset);
    }
    memcpy(buf, data->data + offset, len);
    offset += len;
}
void Decoding::get(char *name, uint16_t &nlen) {
    nlen = get<uint16_t, 2>();
    get(name, (size_t)nlen);
}

void Decoding::get(char *name, uint16_t &nlen, char *value, uint16_t &vlen) {
    get(name, nlen);
    get(value, vlen);
}
void Decoding::get(char *name, uint16_t &nlen, uint32_t &vlen) {
    get(name, nlen);
    vlen = get<uint32_t, 4>();
}
Data Decoding::getstring(std::size_t len) {
    if (len) {
        return getdata(len, true);
    } else {
        return getdata(get<uint16_t, 2>(), true);
    }
}

Data Decoding::getdata(size_t len, bool iss) {
    Data val = BuildData(len, iss);
    get(val->data, val->len);
    return val;
}

void Decoding::inflate() {
    if (data->inflate(offset)) {
        throw Fail("Decoding inflate fail with offset(%ld)", offset);
    }
}
std::string addr_cs(Address &addr) {
    std::stringstream ss;
    ss << boost::asio::ip::address_v4(addr.first).to_string();
    ss << ":" << addr.second;
    return ss.str();
}
}
}
