//
//  sqlite_test.hpp
//  emule-x
//
//  Created by Centny on 1/19/17.
//
//

#ifndef sqlite_test_h
#define sqlite_test_h
#include "../../include/emulex/db/sqlite.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(sqlite)  // name of the test suite is stringtest

#define TEST_SQL \
    "\
create table xxx\
(\
name                 varchar(32) not null,\
value                varchar(10240),\
value2               blob,\
primary key (name)\
);\
"
using namespace emulex::db;

BOOST_AUTO_TEST_CASE(db) {
    boost::filesystem::remove_all(boost::filesystem::path("t.db"));
    boost::filesystem::remove_all(boost::filesystem::path("t2.db"));
    auto bdata = emulex::db::BuildData(3);
    bdata->data[1] = 1;
    //
    std::map<int, const char*> vsql;
    vsql[0] = TEST_SQL;
    {  // test crud
        //
        auto lite = emulex::db::SQLite(new emulex::db::SQLite_(1));
        lite->init("t.db", vsql);
        {  // test version
            auto ver = lite->version();
            BOOST_CHECK_EQUAL(ver, 1);
        }
        {  // test crud
            long c1;
            c1 = lite->intv("select count(*) from xxx");
            BOOST_CHECK_EQUAL(c1, 0);
            lite->exec("insert into xxx (name,value) values ('%s','%s')", "name", "vv");
            c1 = lite->intv("select count(*) from xxx");
            BOOST_CHECK_EQUAL(c1, 1);
            lite->exec("update xxx set value='%s' where name='%s'", "123", "name");
            c1 = lite->intv("select count(*) from xxx");
            BOOST_CHECK_EQUAL(c1, 1);
            BOOST_CHECK_EQUAL(lite->intv("select value from xxx"), 123);
            BOOST_CHECK_EQUAL(lite->floatv("select value from xxx"), 123);
            auto sval = lite->stringv("select value from xxx");
            BOOST_CHECK_EQUAL(sval->cmp("123", 3), true);
            auto bval = lite->blobv("select value from xxx");
            BOOST_CHECK_EQUAL(bval->cmp("123", 3), true);
            lite->exec(bdata, "update xxx set value2=? where name='%s'", "name");
            auto bval2 = lite->blobv("select value2 from xxx");
            BOOST_CHECK_EQUAL(bval2->cmp(bdata), true);
        }
        //
        // test prepare
        {
            lite->exec("delete from xxx");
            lite->exec("insert into xxx (name,value) values ('%s','%s')", "name", "123");
            lite->exec(bdata, "update xxx set value2=? where name='%s'", "name");
            auto stmt = lite->prepare("select name,value,value2 from xxx");
            while (stmt->step()) {
                BOOST_CHECK_EQUAL(stmt->intv(1), 123);
                BOOST_CHECK_EQUAL(stmt->floatv(1), 123);
                auto sval1 = stmt->stringv(0);
                sval1->print();
                BOOST_CHECK_EQUAL(sval1->cmp("name", 4), true);
                auto bval1 = stmt->blobv(2);
                bval1->print();
                BOOST_CHECK_EQUAL(bval1->cmp(bdata), true);
            }
        }
        //
        // test bind
        {
            lite->exec("delete from xxx");
            lite->exec("insert into xxx (name,value) values ('%s','%s')", "name", "123");
            lite->exec(bdata, "update xxx set value2=? where name='%s'", "name");
            {  // bind char*
                auto ustmt = lite->prepare("update xxx set value=? where name='%s'", "name");
                auto val = BuildData("124", 3, true);
                ustmt->bind(1, val);
                ustmt->step();
                auto stmt = lite->prepare("select name,value,value2 from xxx");
                while (stmt->step()) {
                    BOOST_CHECK_EQUAL(stmt->intv(1), 124);
                    BOOST_CHECK_EQUAL(stmt->floatv(1), 124);
                }
            }
            {  // bind blob
                auto ustmt = lite->prepare("update xxx set value2=? where name='%s'", "name");
                auto val = BuildData("125", 3, false);
                ustmt->bind(1, val);
                ustmt->step();
                auto stmt = lite->prepare("select name,value,value2 from xxx");
                while (stmt->step()) {
                    auto bval1 = stmt->blobv(2);
                    bval1->print();
                    BOOST_CHECK_EQUAL(bval1->cmp("125", 3), true);
                }
            }
            {  // bind int
                auto ustmt = lite->prepare("update xxx set value=? where name='%s'", "name");
                ustmt->bind(1, 126);
                ustmt->step();
                auto stmt = lite->prepare("select name,value,value2 from xxx");
                while (stmt->step()) {
                    BOOST_CHECK_EQUAL(stmt->intv(1), 126);
                    BOOST_CHECK_EQUAL(stmt->floatv(1), 126);
                }
            }
            {  // bind int64
                auto ustmt = lite->prepare("update xxx set value=? where name='%s'", "name");
                ustmt->bind(1, (sqlite3_int64)127);
                ustmt->step();
                auto stmt = lite->prepare("select name,value,value2 from xxx");
                while (stmt->step()) {
                    BOOST_CHECK_EQUAL(stmt->intv(1), 127);
                    BOOST_CHECK_EQUAL(stmt->floatv(1), 127);
                }
            }
            {  // bind double
                auto ustmt = lite->prepare("update xxx set value=? where name='%s'", "name");
                ustmt->bind(1, (double)128);
                ustmt->step();
                auto stmt = lite->prepare("select name,value,value2 from xxx");
                while (stmt->step()) {
                    BOOST_CHECK_EQUAL(stmt->intv(1), 128);
                    BOOST_CHECK_EQUAL(stmt->floatv(1), 128);
                }
            }
            {  // bind null
                auto ustmt = lite->prepare("update xxx set value2=? where name='%s'", "name");
                ustmt->bind(1);
                ustmt->step();
                auto stmt = lite->prepare("select name,value,value2 from xxx");
                while (stmt->step()) {
                    auto bval1 = stmt->blobv(2);
                    bval1->print();
                    BOOST_CHECK_EQUAL(bval1->len, 0);
                }
            }
        }
        //
        // test prepare insert
        {
            lite->exec("delete from xxx");
            auto ustmt = lite->prepare("insert into xxx (name,value) values (?,?)");
            auto name = BuildData("name", 4, true);
            auto val = BuildData("130", 3, true);
            ustmt->bind(1, name);
            ustmt->bind(2, val);
            ustmt->step();
            auto stmt = lite->prepare("select name,value,value2 from xxx");
            while (stmt->step()) {
                BOOST_CHECK_EQUAL(stmt->intv(1), 130);
                BOOST_CHECK_EQUAL(stmt->floatv(1), 130);
            }
        }
    }
    {  // test open error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("ss/ss/t.db", vsql);
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
    }
    {  // test execscript error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->execscript("xxddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
    }
    {  // test exec error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->exec("xddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
    }
    {  // test blob exec error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            auto blob = BuildData("abc", 3);
            tx->exec(blob, "xddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
    }
    {  // test intv error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->intv("xddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->intv("select value from xxx");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
            BOOST_CHECK_EQUAL(f.code, 404);
        }
    }
    {  // test floatv error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->floatv("xddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->floatv("select value from xxx");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.what());
            BOOST_CHECK_EQUAL(f.code, 404);
        }
    }
    {  // test stringv error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->stringv("xddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->stringv("select value from xxx");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
            BOOST_CHECK_EQUAL(f.code, 404);
        }
    }
    {  // test blobv error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->blobv("xddd");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->blobv("select value from xxx");
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
            BOOST_CHECK_EQUAL(f.code, 404);
        }
    }
    {  // test prepare error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            tx->prepare("selectxxname,value,value2 from xxx");
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
    }
    {  // test version error
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            std::map<int, const char*> vsql;
            vsql[0] = TEST_SQL;
            tx->init("t2.db", vsql);
            tx->exec("delete from _env_");
            tx->version();
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
        try {
            auto tx = emulex::db::SQLite(new emulex::db::SQLite_(1));
            tx->version();
            BOOST_CHECK_EQUAL(0, 1);
        } catch (Fail_& f) {
            printf("%s\n", f.err);
        }
    }
    {  // test bind error
        auto lite = emulex::db::SQLite(new emulex::db::SQLite_(1));
        lite->init("t.db", vsql);
        auto ustmt = lite->prepare("update xxx set value=? where name='%s'", "name");
        try {
            ustmt->bind(100, bdata);
            BOOST_CHECK_EQUAL(0, 1);
        } catch (...) {
        }
        try {
            ustmt->bind(100, (double)1);
            BOOST_CHECK_EQUAL(0, 1);
        } catch (...) {
        }
        try {
            ustmt->bind(100, 1);
            BOOST_CHECK_EQUAL(0, 1);
        } catch (...) {
        }
        try {
            ustmt->bind(100, (sqlite3_int64)1);
            BOOST_CHECK_EQUAL(0, 1);
        } catch (...) {
        }
        try {
            ustmt->bind(100);
            BOOST_CHECK_EQUAL(0, 1);
        } catch (...) {
        }
    }
    {  // test sqlite err message
        sqlite_emsg(SQLITE_OK);
        sqlite_emsg(SQLITE_ERROR);
        sqlite_emsg(SQLITE_INTERNAL);
        sqlite_emsg(SQLITE_PERM);
        sqlite_emsg(SQLITE_ABORT);
        sqlite_emsg(SQLITE_BUSY);
        sqlite_emsg(SQLITE_LOCKED);
        sqlite_emsg(SQLITE_NOMEM);
        sqlite_emsg(SQLITE_READONLY);
        sqlite_emsg(SQLITE_INTERRUPT);
        sqlite_emsg(SQLITE_IOERR);
        sqlite_emsg(SQLITE_CORRUPT);
        sqlite_emsg(SQLITE_NOTFOUND);
        sqlite_emsg(SQLITE_FULL);
        sqlite_emsg(SQLITE_CANTOPEN);
        sqlite_emsg(SQLITE_PROTOCOL);
        sqlite_emsg(SQLITE_EMPTY);
        sqlite_emsg(SQLITE_SCHEMA);
        sqlite_emsg(SQLITE_TOOBIG);
        sqlite_emsg(SQLITE_CONSTRAINT);
        sqlite_emsg(SQLITE_MISMATCH);
        sqlite_emsg(SQLITE_MISUSE);
        sqlite_emsg(SQLITE_NOLFS);
        sqlite_emsg(SQLITE_AUTH);
        sqlite_emsg(SQLITE_FORMAT);
        sqlite_emsg(SQLITE_RANGE);
        sqlite_emsg(SQLITE_NOTADB);
        sqlite_emsg(SQLITE_NOTICE);
        sqlite_emsg(SQLITE_WARNING);
        sqlite_emsg(SQLITE_ROW);
        sqlite_emsg(SQLITE_DONE);
        sqlite_emsg(10000);
    }
    printf("db test done...");
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* sqlite_test_h */
