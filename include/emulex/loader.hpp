

#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>
#include <boost/foreach.hpp>
#include <memory>
#include <vector>

#include <libed2k/bencode.hpp>
#include <libed2k/log.hpp>
#include <libed2k/session.hpp>
#include <libed2k/session_settings.hpp>
#include <libed2k/util.hpp>
#include <libed2k/alert_types.hpp>
#include <libed2k/file.hpp>
#include <libed2k/search.hpp>
#include <libed2k/peer_connection_handle.hpp>
#include <libed2k/transfer_handle.hpp>
#include <libed2k/io_service.hpp>
#include <libed2k/server_connection.hpp>
#include <libed2k/filesystem.hpp>

namespace emulex {
class loader_;
class db_interface_;
typedef boost::shared_ptr<loader_> loader;
typedef boost::shared_ptr<db_interface_> db_interface;

class host {
   public:
    enum ntype {
        normal = 0,
        ed2k_server = 1,
        kad_node = 2,
    };
    std::string name;
    std::string addr;
    uint16_t port;
    ntype type = normal;
};

//
class db_interface_ {
   public:
    virtual void load_settings(loader ld);
};

class ed2k_session_ {
   public:
    libed2k::fingerprint print;
    libed2k::session_settings settings;
    libed2k::kad_nodes_dat knd;
    std::vector<host> hosts;
    std::vector<libed2k::session*> ses;

   private:
    libed2k::server_alert alert_placeholder;

   public:
    ed2k_session_();
    // load node data from file.
    virtual bool load_nodes(const std::string& filename);
    // start kad network
    virtual void start_kad(libed2k::session& ses, libed2k::kad_nodes_dat& knd);
    // create session by host.
    virtual libed2k::session* create_session(const host& n);
    // init
    virtual void init();

   protected:
    virtual void on_alert(libed2k::alert const& alert);
};

//
class loader_ : public boost::enable_shared_from_this<loader_> {
   public:
    boost::asio::io_service& ios;
    ed2k_session_ ed2k;
    db_interface db;

   public:
    loader_(boost::asio::io_service& ios, db_interface db);
    virtual void init();
};

//
// namespace emulex
}
