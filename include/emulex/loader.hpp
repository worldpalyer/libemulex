

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

class ed2k_session_ {
   public:
    libed2k::fingerprint print;
    libed2k::session_settings settings;
    libed2k::kad_nodes_dat knd;
    libed2k::session* ses;
    //

   private:
    libed2k::server_alert alert_placeholder;

   public:
    ed2k_session_();
    virtual ~ed2k_session_();
    // load node data from file.
    virtual bool load_nodes(const std::string& filename);
    virtual void start();
    virtual void stop();
    virtual void server_connect(const std::string& name, const std::string& host, int port);
    virtual void slave_server_connect(const std::string& name, const std::string& host, int port);
    virtual void add_dht_node(const std::string& host, int port, const std::string& id);
    virtual void search(std::string hash);
    virtual void search(boost::uint64_t nMinSize, boost::uint64_t nMaxSize, unsigned int nSourcesCount,
                        unsigned int nCompleteSourcesCount, const std::string& strFileType,
                        const std::string& strFileExtension, const std::string& strCodec, unsigned int nMediaLength,
                        unsigned int nMediaBitrate, const std::string& strQuery);

   protected:
    virtual void on_alert(libed2k::alert const& alert);
    virtual void on_server_initialized(libed2k::server_connection_initialized_alert* alert);
    virtual void on_server_resolved(libed2k::server_name_resolved_alert* alert);
    virtual void on_server_status(libed2k::server_status_alert* alert);
    virtual void on_server_message(libed2k::server_message_alert* alert);
    virtual void on_server_identity(libed2k::server_identity_alert* alert);
    //
    virtual void on_server_shared(libed2k::shared_files_alert* alert);
    virtual void on_shutdown_completed();
};

enum HashType {
    MD4_H = 1,
    MD5_H = 2,
    SHA1_H = 3,
};
//
class loader_ : public ed2k_session_ {
    //   public:
    //    boost::shared_ptr<ed2k_session_> ed2k;

   public:
    loader_();
    virtual void search_file(std::string hash, HashType type);
    virtual void search_file(std::string query, std::string extension = "", boost::uint64_t min_size = 0,
                             boost::uint64_t max_size = 0);
};

//
// namespace emulex
}
