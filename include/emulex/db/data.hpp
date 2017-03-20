//
//  data.hpp
//  boost-utils
//
//  Created by Centny on 1/22/17.
//
//

#ifndef data_hpp
#define data_hpp
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
namespace emulex {
namespace db {
/*
 the char data manager by shared ptr.
 */
class Data_;
typedef boost::shared_ptr<Data_> Data;
class Data_ {
   public:
    char *data;
    size_t len;
    bool iss;

   public:
    Data_(size_t len, bool iss = false);
    Data_(const char *buf, size_t len, bool iss = false);
    virtual ~Data_();
    virtual char at(size_t i);
    virtual void print(char *buf = 0);
    virtual Data sub(size_t offset, size_t len, bool iss = false);
    virtual bool cmp(const char *val, size_t len = 0);
    virtual bool cmp(Data &data);
    virtual bool cmp(Data_ *data);
    virtual int inflate(size_t offset = 0);
    virtual int deflate(size_t offset = 0);
};
Data BuildData(const char *buf, size_t len, bool iss = false);
Data BuildData(size_t len, bool iss = false);
Data JoinData(Data &a, Data &b);
Data FromHex(const char *hex);
int hex2int(char input);
struct DataComparer {
    bool operator()(const Data &first, const Data &second) const;
};
//
}
}
#endif /* data_hpp */
