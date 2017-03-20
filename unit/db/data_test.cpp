//
//  data_test.hpp
//  boost-utils
//
//  Created by Centny on 1/22/17.
//
//

#ifndef data_test_h
#define data_test_h
#include <vector>
#include "../../include/emulex/db/data.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(data)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(sub) {
    for (int j = 0; j < 100; j++) {  // test sub
        auto a = emulex::db::BuildData("abc", 3, true);
        a->print();
        BOOST_CHECK_EQUAL(a->at(0), 'a');
        BOOST_CHECK_EQUAL(a.use_count(), 1);
        for (int i = 0; i < 10; i++) {
            //
            {
                auto b = a->sub(0, 1, true);
                b->print();
                // printf("xx->%p\n", b.get());
                BOOST_CHECK_EQUAL(b->at(0), 'a');
                BOOST_CHECK_EQUAL(b.use_count(), 1);
            }
            //
            {
                auto c = a->sub(2, 1, true);
                c->print();
                // printf("xx->%p\n", c.get());
                BOOST_CHECK_EQUAL(c->at(0), 'c');
                BOOST_CHECK_EQUAL(c.use_count(), 1);
            }
            //
        }
    }
}

BOOST_AUTO_TEST_CASE(cmp) {
    auto a = emulex::db::BuildData("abc", 3, true);
    auto b = a->sub(0, 1, true);
    BOOST_CHECK_EQUAL(a->cmp(b), false);
    auto c = emulex::db::BuildData("abc", 3, true);
    BOOST_CHECK_EQUAL(a->cmp(c.get()), true);
    auto d = emulex::db::BuildData("cde", 3, true);
    BOOST_CHECK_EQUAL(a->cmp(d.get()), false);
}

BOOST_AUTO_TEST_CASE(cmp2) {
    auto a = emulex::db::BuildData("abc", 3, false);
    auto b = a->sub(0, 1, false);
    BOOST_CHECK_EQUAL(a->cmp(b), false);
    auto c = emulex::db::BuildData("abc", 3, false);
    BOOST_CHECK_EQUAL(a->cmp(c.get()), true);
    auto d = emulex::db::BuildData("cde", 3, false);
    BOOST_CHECK_EQUAL(a->cmp(d.get()), false);
    auto e = emulex::db::BuildData(3, false);
    e->len = 0;
    BOOST_CHECK_EQUAL(e->cmp(e), false);
    emulex::db::BuildData(3, true);
}

BOOST_AUTO_TEST_CASE(flate) {
    const char* data =
        "abcdefgh1234567890abcdefgh1234567890abcdefgh1234567890abcdefgh1234567890abcdefgh1234567890abcdefgh1234567890ab"
        "cdefgh1234567890abcdefgh1234567890abcdefgh1234567890abcdefgh1234567890";

    auto def = emulex::db::BuildData(data, strlen(data), false);
    def->print();
    BOOST_CHECK_EQUAL(def->cmp(data, strlen(data)), true);
    BOOST_CHECK_EQUAL(def->deflate(3), 0);
    def->print();
    BOOST_CHECK_EQUAL(def->len < strlen(data), true);
    BOOST_CHECK_EQUAL(def->at(2), 'c');
    BOOST_CHECK_EQUAL(def->cmp(data, strlen(data)), false);
    BOOST_CHECK_EQUAL(def->inflate(3), 0);
    def->print();
    BOOST_CHECK_EQUAL(def->cmp(data, strlen(data)), true);
    {  // test inflate error
        BOOST_CHECK_EQUAL(def->inflate(3) != 0, true);
        BOOST_CHECK_EQUAL(def->inflate(def->len) != 0, true);
        memset(def->data, 0, def->len);
        BOOST_CHECK_EQUAL(def->inflate(def->len) != 0, true);
    }
    {  // test deflate error
        BOOST_CHECK_EQUAL(def->deflate(3), 0);
        BOOST_CHECK_EQUAL(def->deflate(3) != 0, true);
        BOOST_CHECK_EQUAL(def->deflate(def->len) != 0, true);
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* data_test_h */
