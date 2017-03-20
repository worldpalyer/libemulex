//
//  sqlite.hpp
//  boost-utils
//
//  Created by Centny on 1/22/17.
//
//

#ifndef sqlite_hpp
#define sqlite_hpp

#include <sqlite3.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <map>
#include <string>
#include "data.hpp"
#include "fail.hpp"

namespace emulex {
namespace db {
// the sqlite error message by sqlite code.
const char* sqlite_emsg(int code);
/*
 the sqlite prepare stmt for query mulit row data.
 */
class STMT_ {
   protected:
    sqlite3* db;
    sqlite3_stmt* stmt;
    Data blob;

   public:
    STMT_(sqlite3* db);
    virtual ~STMT_();
    virtual void finalize();
    virtual void prepare(const char* sql);
    //    virtual void prepare(Data& blob, const char* sql);
    virtual bool step();
    virtual long intv(int idx);
    virtual double floatv(int idx);
    virtual Data stringv(int idx);
    virtual Data blobv(int idx);
    //
    virtual void bind(int idx, Data& val);
    virtual void bind(int idx, double val);
    virtual void bind(int idx, int val);
    virtual void bind(int idx, sqlite3_int64 val);
    virtual void bind(int idx);
};
typedef boost::shared_ptr<STMT_> STMT;
/*
 the sqlite db manager which provider db version manager and multi tools to crud.
 */
class SQLite_ {
   protected:
    sqlite3* db;
    int cver;
    char buf[512000];

   public:
    SQLite_(int cver);
    virtual ~SQLite_();
    virtual void init(const char* spath, std::map<int, const char*> vsql);
    virtual int version();
    virtual void execscript(const char* sql);
    virtual void exec(const char* fmt, ...);
    virtual void exec(Data& blob, const char* fmt, ...);
    virtual long intv(const char* fmt, ...);
    virtual double floatv(const char* fmt, ...);
    virtual Data stringv(const char* fmt, ...);
    virtual Data blobv(const char* fmt, ...);
    //
    virtual STMT prepare(const char* fmt, ...);
    //    virtual STMT prepare(Data& blob, const char* fmt, ...);
};
typedef boost::shared_ptr<SQLite_> SQLite;
//
}
}

#endif /* sqlite_hpp */
