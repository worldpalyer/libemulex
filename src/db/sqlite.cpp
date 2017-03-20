//
//  sqlite.cpp
//  boost-utils
//
//  Created by Centny on 1/22/17.

#include "../include/emulex/db/sqlite.hpp"
#include <vector>

namespace emulex {
namespace db {

#define VER_SQL \
    "\
/*==============================================================*/\
/* Table: _env                                                */\
/*==============================================================*/\
create table _env_\
(\
name                 varchar(32) not null,\
value                varchar(10240),\
primary key (name)\
);\
insert into _env_ (name, value) values ('version', %d );\
"
const char* sqlite_emsg(int code) {
    switch (code) {
        case SQLITE_OK:
            return "SQLITE_OK";
        case SQLITE_ERROR:
            return "SQLITE_ERROR";
        case SQLITE_INTERNAL:
            return "SQLITE_INTERNAL";
        case SQLITE_PERM:
            return "SQLITE_PERM";
        case SQLITE_ABORT:
            return "SQLITE_ABORT";
        case SQLITE_BUSY:
            return "SQLITE_BUSY";
        case SQLITE_LOCKED:
            return "SQLITE_LOCKED";
        case SQLITE_NOMEM:
            return "SQLITE_NOMEM";
        case SQLITE_READONLY:
            return "SQLITE_READONLY";
        case SQLITE_INTERRUPT:
            return "SQLITE_INTERRUPT";
        case SQLITE_IOERR:
            return "SQLITE_IOERR";
        case SQLITE_CORRUPT:
            return "SQLITE_CORRUPT";
        case SQLITE_NOTFOUND:
            return "SQLITE_NOTFOUND";
        case SQLITE_FULL:
            return "SQLITE_FULL";
        case SQLITE_CANTOPEN:
            return "SQLITE_CANTOPEN";
        case SQLITE_PROTOCOL:
            return "SQLITE_PROTOCOL";
        case SQLITE_EMPTY:
            return "SQLITE_EMPTY";
        case SQLITE_SCHEMA:
            return "SQLITE_SCHEMA";
        case SQLITE_TOOBIG:
            return "SQLITE_TOOBIG";
        case SQLITE_CONSTRAINT:
            return "SQLITE_CONSTRAINT";
        case SQLITE_MISMATCH:
            return "SQLITE_MISMATCH";
        case SQLITE_MISUSE:
            return "SQLITE_MISUSE";
        case SQLITE_NOLFS:
            return "SQLITE_NOLFS";
        case SQLITE_AUTH:
            return "SQLITE_AUTH";
        case SQLITE_FORMAT:
            return "SQLITE_FORMAT";
        case SQLITE_RANGE:
            return "SQLITE_RANGE";
        case SQLITE_NOTADB:
            return "SQLITE_NOTADB";
        case SQLITE_NOTICE:
            return "SQLITE_NOTICE";
        case SQLITE_WARNING:
            return "SQLITE_WARNING";
        case SQLITE_ROW:
            return "SQLITE_ROW";
        case SQLITE_DONE:
            return "SQLITE_DONE";
        default:
            return "unknow";
    }
}

STMT_::STMT_(sqlite3* db) : db(db), stmt(0) {}

STMT_::~STMT_() { finalize(); }

void STMT_::finalize() {
    if (stmt) {
        sqlite3_finalize(stmt);
        stmt = 0;
    }
}

void STMT_::prepare(const char* sql) {
    int res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(sql) + 64, "STMT_ prepare by sql(%s) fail with %s,%s", sql, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
}

// void STMT_::prepare(Data& blob, const char* sql) {
//    int res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//    if (res != SQLITE_OK) {
//        throw LFail(strlen(sql) + 64, "STMT_ prepare by sql(%s) fail with %s", sql, sqlite_emsg(res));
//    }
//    this->blob = blob;
//    sqlite3_bind_blob(stmt, 1, blob->data, blob->len, NULL);
//}

bool STMT_::step() {
    int res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
        return true;
    } else if (res == SQLITE_DONE || res == SQLITE_OK) {
        return false;
    } else {
        throw Fail("STMT_ step fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
    }
}

long STMT_::intv(int idx) { return sqlite3_column_int64(stmt, idx); }

double STMT_::floatv(int idx) { return sqlite3_column_double(stmt, idx); }

Data STMT_::stringv(int idx) {
    auto text = sqlite3_column_text(stmt, idx);
    if (text) {
        return BuildData((const char*)text, strlen((const char*)text), true);
    } else {
        return Data();
    }
}

Data STMT_::blobv(int idx) {
    auto blob = sqlite3_column_blob(stmt, idx);
    if (blob) {
        auto len = sqlite3_column_bytes(stmt, idx);
        return BuildData((const char*)blob, len);
    } else {
        return Data();
    }
}

void STMT_::bind(int idx, Data& val) {
    int res;
    if (val->iss) {
        res = sqlite3_bind_text(stmt, idx, val->data, val->len, NULL);
    } else {
        res = sqlite3_bind_blob(stmt, idx, val->data, val->len, NULL);
    }
    if (res != SQLITE_OK) {
        throw Fail("STMT_ bind data(%lu,%d) fail with %s,%s", val->len, val->iss, sqlite_emsg(res), sqlite3_errmsg(db));
    }
}
void STMT_::bind(int idx, double val) {
    int res = sqlite3_bind_double(stmt, idx, val);
    if (res != SQLITE_OK) {
        throw Fail("STMT_ bind double fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
    }
}
void STMT_::bind(int idx, int val) {
    int res = sqlite3_bind_int(stmt, idx, val);
    if (res != SQLITE_OK) {
        throw Fail("STMT_ bind int fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
    }
}
void STMT_::bind(int idx, sqlite3_int64 val) {
    int res = sqlite3_bind_int64(stmt, idx, val);
    if (res != SQLITE_OK) {
        throw Fail("STMT_ bind int64 fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
    }
}
void STMT_::bind(int idx) {
    int res = sqlite3_bind_null(stmt, idx);
    if (res != SQLITE_OK) {
        throw Fail("STMT_ bind null fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
    }
}

SQLite_::SQLite_(int cver) {
    this->db = 0;
    this->cver = cver;
}

SQLite_::~SQLite_() {
    if (db) {
        sqlite3_free(db);
        db = 0;
    }
}

void SQLite_::init(const char* spath, std::map<int, const char*> vsql) {
    int res = sqlite3_open(spath, &db);
    if (res != SQLITE_OK) {
        throw LFail(strlen(spath) + 64, "SQLite_ open db(%s) fail with %s,%s", spath, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
    int ver = version();
    if (ver == 0) {
        execscript(vsql[0]);
        return;
    }
}

int SQLite_::version() {
    int ver, res;
    sqlite3_stmt* stmt;
    const char* emsg = NULL;
    const char* query = "select value from _env_ where name = 'version'";
    res = sqlite3_prepare_v2(db, query, -1, &stmt, &emsg);
    if (res == SQLITE_OK) {
        res = sqlite3_step(stmt);
        if (res == SQLITE_ROW) {
            ver = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return ver;
        } else {
            sqlite3_finalize(stmt);
            throw Fail("SQLite_ get version row fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
        }
    }
    if (res != SQLITE_ERROR) {
        throw Fail("SQLite_ get version fail with %s,%s", sqlite_emsg(res), sqlite3_errmsg(db));
    }
    sprintf(buf, VER_SQL, cver);
    execscript(buf);
    return 0;
}

void SQLite_::execscript(const char* sql) {
    int res;
    std::string tsql = boost::regex_replace(std::string(sql), boost::regex("/\\*[^\\*]*\\*/"), std::string(""));
    std::vector<std::string> sqls;
    boost::split(sqls, tsql, boost::is_any_of(";"));
    BOOST_FOREACH (std::string& s, sqls) {
        boost::trim(s);
        if (s.size() < 1) {
            continue;
        }
        res = sqlite3_exec(db, s.c_str(), NULL, NULL, NULL);
        if (res != SQLITE_OK) {
            throw LFail(strlen(sql) + 64, "SQLite_ execute sql(%s) fail with %s,%s", s.c_str(), sqlite_emsg(res),
                        sqlite3_errmsg(db));
        }
    }
}

void SQLite_::exec(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    auto res = sqlite3_exec(db, buf, NULL, NULL, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(buf) + 64, "SQLite_ update by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
}

void SQLite_::exec(Data& blob, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(db, buf, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(buf) + 64, "SQLite_ update by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
    sqlite3_bind_blob(stmt, 1, blob->data, blob->len, NULL);
    res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (res != SQLITE_DONE) {
        throw LFail(strlen(buf) + 64, "SQLite_ update by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
}

long SQLite_::intv(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(db, buf, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(buf) + 64, "SQLite_ intv by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
        long val = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return val;
    } else {
        sqlite3_finalize(stmt);
        throw CFail(404);
    }
}

double SQLite_::floatv(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(db, buf, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(buf) + 64, "SQLite_ floatev by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
        int val = sqlite3_column_double(stmt, 0);
        sqlite3_finalize(stmt);
        return val;
    } else {
        sqlite3_finalize(stmt);
        throw CFail(404);
    }
}

Data SQLite_::stringv(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(db, buf, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(buf) + 64, "SQLite_ stringv by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
        auto text = sqlite3_column_text(stmt, 0);
        auto data = BuildData((const char*)text, strlen((const char*)text));
        sqlite3_finalize(stmt);
        return data;
    } else {
        sqlite3_finalize(stmt);
        throw CFail(404);
    }
}

Data SQLite_::blobv(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(db, buf, -1, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw LFail(strlen(buf) + 64, "SQLite_ blobv by sql(%s) fail with %s,%s", buf, sqlite_emsg(res),
                    sqlite3_errmsg(db));
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
        auto blob = sqlite3_column_blob(stmt, 0);
        auto len = sqlite3_column_bytes(stmt, 0);
        auto data = BuildData((const char*)blob, len);
        sqlite3_finalize(stmt);
        return data;
    } else {
        sqlite3_finalize(stmt);
        throw CFail(404);
    }
}

STMT SQLite_::prepare(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    auto stmt = STMT(new STMT_(db));
    stmt->prepare(buf);
    return stmt;
}

// STMT SQLite_::prepare(Data& blob, const char* fmt, ...) {
//    va_list args;
//    va_start(args, fmt);
//    vsprintf(buf, fmt, args);
//    va_end(args);
//    auto stmt = STMT(new STMT_(db));
//    stmt->prepare(blob, buf);
//    return stmt;
//}
}
}
