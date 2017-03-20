//
// Created by Centny on 3/16/17.
//
#define BOOST_TEST_MODULE main

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_suite2)

BOOST_AUTO_TEST_CASE(test_case3) { BOOST_TEST(true); }

BOOST_AUTO_TEST_CASE(test_case4) { BOOST_TEST(false); }

BOOST_AUTO_TEST_SUITE_END()
