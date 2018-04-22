

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

// load node data from file.
bool load_nodes(libed2k::kad_nodes_dat& knd, const std::string& filename);
bool load_raw_nodes(libed2k::kad_nodes_dat& knd, char* raw, size_t length);
bool load_server_met(libed2k::server_met& sm, const std::string& filename);
bool load_raw_server_met(libed2k::server_met& sm, char* raw, size_t length);
class ed2k_session_ {
   public:
    libed2k::fingerprint print;
    libed2k::session_settings settings;
    libed2k::kad_nodes_dat knd;
    libed2k::session* ses;
    boost::posix_time::time_duration fs_timer_delay;
    //
   protected:
    boost::thread* timer_thr;
    libed2k::io_service timer_ios;
    boost::asio::deadline_timer fs_timer;

   private:
    libed2k::server_alert alert_placeholder;

   public:
    ed2k_session_();
    virtual ~ed2k_session_();
    virtual void start(bool upnp);
    virtual void stop();
    virtual void server_connect(const std::string& name, const std::string& host, int port);
    virtual void slave_server_connect(const std::string& name, const std::string& host, int port);
    virtual void add_dht_node(const std::string& host, int port, const std::string& id);
    virtual void search(std::string hash);
    virtual void search(boost::uint64_t nMinSize, boost::uint64_t nMaxSize, unsigned int nSourcesCount,
                        unsigned int nCompleteSourcesCount, const std::string& strFileType,
                        const std::string& strFileExtension, const std::string& strCodec, unsigned int nMediaLength,
                        unsigned int nMediaBitrate, const std::string& strQuery);
    virtual libed2k::transfer_handle add_transfer(const std::string& hash, const std::string& path,
                                                  boost::uint64_t size,
                                                  const std::vector<std::string>& parts = std::vector<std::string>(),
                                                  const std::string& resources = "", bool seed = false);
    virtual std::vector<libed2k::transfer_handle> list_transfter();
    virtual libed2k::transfer_handle find_transfer(libed2k::md4_hash& hash);
    virtual libed2k::transfer_handle restore_transfer(std::string path);
    virtual libed2k::transfer_handle pause_transfer(libed2k::md4_hash& hash);
    virtual libed2k::transfer_handle resume_transfer(libed2k::md4_hash& hash);
    virtual libed2k::transfer_handle remove_transfer(libed2k::md4_hash& hash);

   protected:
    virtual void on_alert(libed2k::alert const& alert);
    virtual void on_server_initialized(libed2k::server_connection_initialized_alert* alert);
    virtual void on_server_resolved(libed2k::server_name_resolved_alert* alert);
    virtual void on_server_status(libed2k::server_status_alert* alert);
    virtual void on_server_message(libed2k::server_message_alert* alert);
    virtual void on_server_identity(libed2k::server_identity_alert* alert);
    virtual void on_server_connection_closed(libed2k::server_connection_closed* alert);
    //
    virtual void on_server_shared(libed2k::shared_files_alert* alert);
    virtual void on_finished_transfer(libed2k::finished_transfer_alert* alert);
    virtual void on_save_resume_data_transfer(libed2k::save_resume_data_alert* alert);
    virtual void on_resumed_data_transfer(libed2k::resumed_transfer_alert* alert);
    virtual void on_paused_data_transfer(libed2k::paused_transfer_alert* alert);
    virtual void on_deleted_data_transfer(libed2k::deleted_transfer_alert* alert);
    virtual void on_state_changed(libed2k::state_changed_alert* alert);
    virtual void on_transfer_added(libed2k::added_transfer_alert* alert);
    virtual void on_portmap(libed2k::portmap_alert* alert);
    virtual void on_portmap_error(libed2k::portmap_error_alert* alert);
    virtual void on_shutdown_completed();
    virtual void save_fast_resume(const boost::system::error_code& ec);
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
    virtual ~loader_();
    virtual void search_file(std::string hash, HashType type);
    virtual void search_file(std::string query, std::string extension = "", boost::uint64_t min_size = 0,
                             boost::uint64_t max_size = 0);
};

//
// namespace emulex
}  // namespace emulex
