//
//  data.cpp
//  boost-utils
//
//  Created by Centny on 1/22/17.
//
//

#include "../include/emulex/db/data.hpp"
#include <zlib.h>
#include <stdio.h>
#include <string.h>
namespace emulex {
namespace db {

// int zinflate(const void *src, size_t srcLen, void *dst, size_t dstLen, size_t &out) {
//    z_stream strm = {0};
//    strm.total_in = strm.avail_in = srcLen;
//    strm.total_out = strm.avail_out = dstLen;
//    strm.next_in = (Bytef *)src;
//    strm.next_out = (Bytef *)dst;
//    strm.zalloc = Z_NULL;
//    strm.zfree = Z_NULL;
//    strm.opaque = Z_NULL;
//    int err = -1;
//    err = inflateInit2(&strm, (15 + 32));  // 15 window bits, and the +32 tells zlib to to detect if using gzip or
//    zlib
//    if (err != Z_OK) {
//        inflateEnd(&strm);
//        return err;
//    }
//    err = inflate(&strm, Z_FINISH);
//    if (err == Z_STREAM_END) {
//        out = strm.total_out;
//        err = 0;
//    }
//    inflateEnd(&strm);
//    return err;
//}
//
// int zdeflate(const void *src, size_t srcLen, void *dst, size_t dstLen, size_t &out) {
//    z_stream strm = {0};
//    strm.total_in = strm.avail_in = srcLen;
//    strm.total_out = strm.avail_out = dstLen;
//    strm.next_in = (Bytef *)src;
//    strm.next_out = (Bytef *)dst;
//    strm.zalloc = Z_NULL;
//    strm.zfree = Z_NULL;
//    strm.opaque = Z_NULL;
//    int err = -1;
//    err = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
//    if (err != Z_OK) {
//        deflateEnd(&strm);
//        return err;
//    }
//    err = deflate(&strm, Z_FINISH);
//    if (err == Z_STREAM_END) {
//        out = strm.total_out;
//        err = 0;
//    }
//    deflateEnd(&strm);
//    return err;
//}

Data_::Data_(size_t len, bool iss) {
    if (iss) {
        data = new char[len + 1];
        memset(data, 0, len + 1);
        this->iss = iss;
    } else {
        data = new char[len];
        memset(data, 0, len);
    }
    this->len = len;
}

Data_::Data_(const char *buf, size_t len, bool iss) {
    if (iss) {
        data = new char[len + 1];
        data[len] = 0;
        this->iss = iss;
    } else {
        data = new char[len];
    }
    memcpy(data, buf, len);
    this->len = len;
}

Data_::~Data_() {
    //    printf("%p->%s->%d\n", this, "Data_ free...", del);
    delete[] data;
}

char Data_::at(size_t i) { return data[i]; }

void Data_::print(char *buf) {
    char *tbuf = buf;
    if (tbuf == 0) {
        tbuf = new char[32 + 3 * len];
    }
    size_t tlen = sprintf(tbuf, "Data(%ld):", len);
    for (size_t i = 0; i < len; i++) {
        tlen += sprintf(tbuf + tlen, "%02x ", (unsigned char)data[i]);
    }
    tbuf[tlen] = 0;
    if (buf == 0) {
        printf("%s\n", tbuf);
        delete[] tbuf;
    }
}

Data Data_::sub(size_t offset, size_t len, bool iss) { return BuildData(data + offset, len, iss); }

bool Data_::cmp(const char *val, size_t len) {
    if (len < 1) {
        len = strlen(val);
    }
    if (this->len != len) {
        return false;
    }
    if (len) {
        return memcmp(this->data, val, len) == 0;
    } else {
        return false;
    }
}
bool Data_::cmp(Data &data) { return cmp(data->data, data->len); }
bool Data_::cmp(Data_ *data) { return cmp(data->data, data->len); }

int Data_::inflate(size_t offset) {
    z_stream strm = {0};
    strm.total_in = strm.avail_in = len - offset;
    strm.next_in = (Bytef *)data + offset;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.total_out = 0;
    int err = -1;
    err = inflateInit2(&strm, (15 + 32));  // 15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
    if (err != Z_OK) {
        inflateEnd(&strm);
        return -1;
    }
    size_t nlen = 2 * len;
    char *ndata = new char[2 * len];
    if (offset) {
        memcpy(ndata, data, offset);
    }
    do {
        strm.next_out = (Bytef *)ndata + offset + strm.total_out;
        strm.avail_out = nlen - offset - strm.total_out;
        err = ::inflate(&strm, Z_NO_FLUSH);
        switch (err) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
            case Z_BUF_ERROR:
                (void)inflateEnd(&strm);
                delete[] ndata;
                return -1;
        }
        if (err == Z_STREAM_END) {
            break;
        }
        char *tdata = new char[nlen + 2 * len];
        memcpy(tdata, ndata, nlen);
        delete[] ndata;
        ndata = tdata;
        nlen += 2 * len;
    } while (true);
    delete data;
    data = ndata;
    len = offset + strm.total_out;
    inflateEnd(&strm);
    return 0;
}

int Data_::deflate(size_t offset) {
    char *ndata = new char[len];
    if (offset) {
        memcpy(ndata, data, offset);
    }
    z_stream strm = {0};
    strm.total_in = strm.avail_in = len - offset;
    strm.total_out = strm.avail_out = len - offset;
    strm.next_in = (Bytef *)data + offset;
    strm.next_out = (Bytef *)ndata + offset;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int err = -1;
    err = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (err != Z_OK) {
        deflateEnd(&strm);
        delete[] ndata;
        return -1;
    }
    err = ::deflate(&strm, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&strm);
        delete[] ndata;
        return -1;
    }
    delete[] data;
    data = ndata;
    len = offset + strm.total_out;
    deflateEnd(&strm);
    return 0;
}

Data BuildData(const char *buf, size_t len, bool iss) { return Data(new Data_(buf, len, iss)); }

Data BuildData(size_t len, bool iss) { return Data(new Data_(len, iss)); }

Data JoinData(Data &a, Data &b) {
    auto data = BuildData(a->len + b->len, a->iss);
    memcpy(data->data, a->data, a->len);
    memcpy(data->data + a->len, b->data, b->len);
    return data;
}

Data DataFromHex(const char *hex) {
    size_t len = strlen(hex);
    if (len < 2 || len % 2) {
        return Data();
    }
    auto bys = BuildData(len / 2);
    for (unsigned int i = 0; i < len; i += 2) {
        bys->data[i / 2] = strtol(hex + (i * 2), NULL, 16);
    }
    return bys;
}

int hex2int(char input) {
    if (input >= '0' && input <= '9') return input - '0';
    if (input >= 'A' && input <= 'F') return input - 'A' + 10;
    if (input >= 'a' && input <= 'f') return input - 'a' + 10;
    printf("%c\n", input);
    throw std::invalid_argument("Invalid input string");
}
bool DataComparer::operator()(const Data &first, const Data &second) const {
    if (first->len != second->len) {
        return first->len < second->len;
    }
    for (size_t i = 0; i < first->len; i++) {
        if (first->data[i] == second->data[i]) {
            continue;
        }
        return first->data[i] < second->data[i];
    }

    return false;
}
//
}
}
