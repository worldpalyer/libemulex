//
// Created by Centny on 3/16/17.
//

#include <emulex/loader.hpp>
//#include <boost/asio.hpp>

int main() {
    LOGGER_INIT_FILE(LOG_ALL, "out.txt");
    // emulex::db_interface db(new emulex::db_interface_);
    boost::asio::io_service ios;
    emulex::loader_ ld;
    ld.start(true);
    // printf("start...\n");
    ios.run();
    // sleep(1000);
    printf("all end...\n");
}
