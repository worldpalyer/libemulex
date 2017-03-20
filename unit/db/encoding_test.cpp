//
//  encoding_test.hpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#ifndef encoding_test_h
#define encoding_test_h
#include "../../include/emulex/db/encoding.hpp"
#include <boost/test/unit_test.hpp>

using namespace emulex::db;

BOOST_AUTO_TEST_SUITE(Encoding)
BOOST_AUTO_TEST_CASE(TesEncoding) {
    emulex::db::Encoding enc;
    //
    enc.reset();
    enc.put("abc", 3);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 3);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), 'a');
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 'b');
    BOOST_CHECK_EQUAL(enc.encode()->at(2), 'c');
    //
    enc.reset();
    enc.put((uint8_t)1);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), 1);
    //
    enc.reset();
    enc.put((uint16_t)1);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 2);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), 1);
    //
    enc.reset();
    enc.put((uint16_t)2);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 2);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), 2);
    //
    enc.reset();
    enc.put((uint32_t)2);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 4);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), 2);
    //
    enc.reset();
    enc.put((uint64_t)2);
    BOOST_CHECK_EQUAL(enc.size(), 8);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), 2);
    //
    enc.reset();
    enc.putv("a", "b");
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 7);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), STR_TAG);
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(3), 'a');
    //
    enc.reset();
    enc.putv(0x01, "b");
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 7);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), STR_TAG);
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(3), 0x01);
    //
    enc.reset();
    enc.putv("a", (uint32_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 8);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), INT_TAG);
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(3), 'a');
    BOOST_CHECK_EQUAL(enc.encode()->at(4), 100);
    //
    enc.reset();
    enc.putv(0x01, (uint32_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 8);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), INT_TAG);
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(3), 0x01);
    BOOST_CHECK_EQUAL(enc.encode()->at(4), 100);
    //
    enc.reset();
    enc.putv("a", (uint64_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 12);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), INT_TAG);
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(3), 'a');
    BOOST_CHECK_EQUAL(enc.encode()->at(4), 100);
    //
    enc.reset();
    enc.putv(0x01, (uint64_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 12);
    BOOST_CHECK_EQUAL(enc.encode()->at(0), INT_TAG);
    BOOST_CHECK_EQUAL(enc.encode()->at(1), 1);
    BOOST_CHECK_EQUAL(enc.encode()->at(3), 0x01);
    BOOST_CHECK_EQUAL(enc.encode()->at(4), 100);
    //
    printf("%s\n", "test encoding done...");
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* encoding_test_h */
