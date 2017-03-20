//
// Created by Centny on 3/16/17.
//

#include <boost/test/unit_test.hpp>

// class
BOOST_AUTO_TEST_SUITE(test_loader)

BOOST_AUTO_TEST_CASE(test_case1) { BOOST_TEST_WARN(sizeof(int) < 4U); }

BOOST_AUTO_TEST_CASE(test_case2) {
    BOOST_TEST_REQUIRE(1 == 2);
    BOOST_FAIL("Should never reach this line");
}

BOOST_AUTO_TEST_SUITE_END()
