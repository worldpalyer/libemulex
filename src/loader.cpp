
#include "emulex/loader.hpp"
#include <fstream>
#include <libed2k/bencode.hpp>

namespace emulex {

bool load_nodes(libed2k::kad_nodes_dat& knd, const std::string& filename) {
    using libed2k::kad_nodes_dat;
    using libed2k::kad_entry;
    DBG("File nodes: " << filename);
    std::ifstream fs(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    if (fs) {
        libed2k::archive::ed2k_iarchive ifa(fs);
        try {
            ifa >> knd;
        } catch (libed2k::libed2k_exception& e) {
            DBG("error on load nodes.dat " << e.what());
            return false;
        }
    } else {
        DBG("unable to open " << filename);
        return false;
    }
    return true;
}

bool load_server_met(libed2k::server_met& sm, const std::string& filename) {
    std::ifstream ifs(filename, std::ios_base::binary);
    if (ifs) {
        libed2k::archive::ed2k_iarchive ifa(ifs);
        try {
            ifa >> sm;
        } catch (libed2k::libed2k_exception& e) {
            DBG("error on load servers.met " << e.what());
            return false;
        }
    } else {
        DBG("unable to open " << filename);
        return false;
    }
    return true;
}

ed2k_session_::ed2k_session_()
    : ses(0),
      fs_timer_delay(boost::posix_time::millisec(10000)),
      fs_timer(timer_ios, fs_timer_delay),
      alert_placeholder("", "", 10) {}

ed2k_session_::~ed2k_session_() {
    if (ses) {
        delete ses;
        ses = 0;
    }
}

void ed2k_session_::start() {
    DBG("ed2k_session_: start initial ed2k session by port:" << settings.listen_port);
    ses = new libed2k::session(print, "0.0.0.0", settings);
    ses->set_alert_mask(libed2k::alert::all_categories);
    ses->set_alert_dispatch(boost::bind(&ed2k_session_::on_alert, this, boost::placeholders::_1));
    timer_thr = new boost::thread(boost::bind(&libed2k::io_service::run, &timer_ios));
}

void ed2k_session_::stop() {
    if (ses) {
        delete ses;
        ses = 0;
        on_shutdown_completed();
    }
    if (timer_thr) {
        timer_ios.stop();
    }
}

void ed2k_session_::server_connect(const std::string& name, const std::string& host, int port) {
    libed2k::server_connection_parameters scp(name, host, port, 60, 60, 60, 60, 60);
    ses->server_connect(scp);
    DBG("ed2k_session_: connect to server " << host << ":" << port);
}

void ed2k_session_::slave_server_connect(const std::string& name, const std::string& host, int port) {
    libed2k::server_connection_parameters scp(name, host, port, 60, 60, 60, 60, 60);
    ses->slave_server_connect(scp);
    DBG("ed2k_session_: connect to server " << host << ":" << port);
}

void ed2k_session_::add_dht_node(const std::string& host, int port, const std::string& id) {
    ses->add_dht_node(std::make_pair(host, port), id);
}

void ed2k_session_::search(std::string hash) {
    libed2k::search_request request = libed2k::generateSearchRequest(libed2k::md4_hash::fromString(hash));
    ses->post_search_request(request);
}

void ed2k_session_::search(boost::uint64_t nMinSize, boost::uint64_t nMaxSize, unsigned int nSourcesCount,
                           unsigned int nCompleteSourcesCount, const std::string& strFileType,
                           const std::string& strFileExtension, const std::string& strCodec, unsigned int nMediaLength,
                           unsigned int nMediaBitrate, const std::string& strQuery) {
    libed2k::search_request request =
        libed2k::generateSearchRequest(nMinSize, nMaxSize, nSourcesCount, nCompleteSourcesCount, strFileType,
                                       strFileExtension, strCodec, nMediaLength, nMediaBitrate, strQuery);
    ses->post_search_request(request);
}

void ed2k_session_::add_transfer(const std::string& hash, const std::string& path, boost::uint64_t size,
                                 const std::vector<std::string>& parts, const std::string& resources, bool seed) {
    libed2k::add_transfer_params param;
    param.file_hash = libed2k::md4_hash::fromString(hash);
    param.file_path = path;
    param.file_size = size;
    param.resources = resources;
    BOOST_FOREACH (const std::string& part, parts) {
        param.piece_hashses.push_back(libed2k::md4_hash::fromString(part));
    }
    param.seed_mode = seed;
    ses->add_transfer(param);
}

std::vector<libed2k::transfer_handle> ed2k_session_::list_transfter() { return ses->get_transfers(); }

bool ed2k_session_::restore(std::string path) {
    std::ifstream ifs(path.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!ifs) {
        return false;
    }
    libed2k::transfer_resume_data trd;
    libed2k::archive::ed2k_iarchive ia(ifs);
    ia >> trd;
    libed2k::add_transfer_params params;
    params.seed_mode = false;
    params.file_path = trd.m_filename.m_collection;
    params.file_size = trd.m_filesize;

    if (trd.m_fast_resume_data.size() > 0) {
        params.resume_data = const_cast<std::vector<char>*>(
            &trd.m_fast_resume_data.getTagByNameId(libed2k::FT_FAST_RESUME_DATA)->asBlob());
    }

    params.file_hash = trd.m_hash;
    ses->add_transfer(params);
    return true;
}

void ed2k_session_::pause(libed2k::md4_hash& hash) {
    std::vector<libed2k::transfer_handle> ts = list_transfter();
    BOOST_FOREACH (const libed2k::transfer_handle& t, ts) {
        if (t.hash() != hash) {
            continue;
        }
        if (t.is_valid()) {
            t.pause();
        }
        return;
    }
}

void ed2k_session_::resume(libed2k::md4_hash& hash) {
    std::vector<libed2k::transfer_handle> ts = list_transfter();
    BOOST_FOREACH (const libed2k::transfer_handle& t, ts) {
        if (t.hash() != hash) {
            continue;
        }
        if (t.is_paused()) {
            t.resume();
        }
        return;
    }
}

void ed2k_session_::on_alert(libed2k::alert const& alert) {
    libed2k::alert* alert_ptr = (libed2k::alert*)&alert;
    if (libed2k::server_connection_initialized_alert* p =
            dynamic_cast<libed2k::server_connection_initialized_alert*>(alert_ptr)) {
        on_server_initialized(p);
    } else if (libed2k::server_name_resolved_alert* p = dynamic_cast<libed2k::server_name_resolved_alert*>(alert_ptr)) {
        on_server_resolved(p);
    } else if (libed2k::server_status_alert* p = dynamic_cast<libed2k::server_status_alert*>(alert_ptr)) {
        on_server_status(p);
    } else if (libed2k::server_message_alert* p = dynamic_cast<libed2k::server_message_alert*>(alert_ptr)) {
        on_server_message(p);
    } else if (libed2k::server_identity_alert* p = dynamic_cast<libed2k::server_identity_alert*>(alert_ptr)) {
        on_server_identity(p);
    } else if (libed2k::shared_files_alert* p = dynamic_cast<libed2k::shared_files_alert*>(alert_ptr)) {
        on_server_shared(p);
    } else if (libed2k::peer_message_alert* p = dynamic_cast<libed2k::peer_message_alert*>(alert_ptr)) {
        DBG("ed2k_session_: MSG: ADDR: " << libed2k::int2ipstr(p->m_np.m_nIP) << " MSG " << p->m_strMessage);
    } else if (libed2k::peer_disconnected_alert* p = dynamic_cast<libed2k::peer_disconnected_alert*>(alert_ptr)) {
        DBG("ed2k_session_: peer disconnected: " << libed2k::int2ipstr(p->m_np.m_nIP));
    } else if (libed2k::peer_captcha_request_alert* p = dynamic_cast<libed2k::peer_captcha_request_alert*>(alert_ptr)) {
        DBG("ed2k_session_: captcha request ");
    } else if (libed2k::peer_captcha_result_alert* p = dynamic_cast<libed2k::peer_captcha_result_alert*>(alert_ptr)) {
        DBG("ed2k_session_: captcha result " << p->m_nResult);
    } else if (libed2k::peer_connected_alert* p = dynamic_cast<libed2k::peer_connected_alert*>(alert_ptr)) {
        DBG("ed2k_session_: peer connected: " << libed2k::int2ipstr(p->m_np.m_nIP) << " status: " << p->m_active);
    } else if (libed2k::shared_files_access_denied* p = dynamic_cast<libed2k::shared_files_access_denied*>(alert_ptr)) {
        DBG("ed2k_session_: peer denied access to shared files: " << libed2k::int2ipstr(p->m_np.m_nIP));
    } else if (libed2k::shared_directories_alert* p = dynamic_cast<libed2k::shared_directories_alert*>(alert_ptr)) {
        DBG("ed2k_session_: peer shared directories: " << libed2k::int2ipstr(p->m_np.m_nIP)
                                                       << " count: " << p->m_dirs.size());
        for (size_t n = 0; n < p->m_dirs.size(); ++n) {
            DBG("ed2k_session_: DIR: " << p->m_dirs[n]);
        }
    } else if (libed2k::save_resume_data_alert* p = dynamic_cast<libed2k::save_resume_data_alert*>(alert_ptr)) {
        on_save_resume_data_transfer(p);
    } else if (dynamic_cast<libed2k::save_resume_data_failed_alert*>(alert_ptr)) {
        DBG("ed2k_session_: save_resume_data_failed_alert");
    } else if (libed2k::transfer_params_alert* p = dynamic_cast<libed2k::transfer_params_alert*>(alert_ptr)) {
        if (!p->m_ec) {
            DBG("ed2k_session_: transfer_params_alert, add transfer for: " << p->m_atp.file_path);
            ses->add_transfer(p->m_atp);
        }
    } else if (libed2k::resumed_transfer_alert* p = dynamic_cast<libed2k::resumed_transfer_alert*>(alert_ptr)) {
        on_resumed_data_transfer(p);
    } else if (libed2k::paused_transfer_alert* p = dynamic_cast<libed2k::paused_transfer_alert*>(alert_ptr)) {
        on_paused_data_transfer(p);
    } else if (libed2k::deleted_transfer_alert* p = dynamic_cast<libed2k::deleted_transfer_alert*>(alert_ptr)) {
        on_deleted_data_transfer(p);
    } else if (libed2k::finished_transfer_alert* p = dynamic_cast<libed2k::finished_transfer_alert*>(alert_ptr)) {
        on_finished_transfer(p);
    } else {
        DBG("ed2k_session_: Unknown alert: " << alert_ptr->message());
    }
}

void ed2k_session_::on_server_initialized(libed2k::server_connection_initialized_alert* alert) {
    DBG("ed2k_session_: server initialized: cid: " << alert->client_id);
}
void ed2k_session_::on_server_resolved(libed2k::server_name_resolved_alert* alert) {
    DBG("ed2k_session_: server name was resolved: " << alert->endpoint);
}
void ed2k_session_::on_server_status(libed2k::server_status_alert* alert) {
    DBG("ed2k_session_: server status: files count: " << alert->files_count << " users count " << alert->users_count);
}
void ed2k_session_::on_server_message(libed2k::server_message_alert* alert) {
    DBG("ed2k_session_: msg: " << alert->server_message);
}
void ed2k_session_::on_server_identity(libed2k::server_identity_alert* alert) {
    DBG("ed2k_session_: server_identity_alert: " << alert->server_hash << " name:  " << alert->server_name
                                                 << " descr: " << alert->server_descr);
}
void ed2k_session_::on_server_shared(libed2k::shared_files_alert* alert) {
    DBG("ed2k_session_: search RESULT: " << alert->m_files.m_collection.size());
}
void ed2k_session_::on_finished_transfer(libed2k::finished_transfer_alert* alert) {
    DBG("ed2k_session_: finished transfer: " << alert->m_handle.save_path());
}
void ed2k_session_::on_save_resume_data_transfer(libed2k::save_resume_data_alert* alert) {
    if (!alert->m_handle.is_valid()) return;
    if (alert->m_handle.is_seed()) return;
    try {
        // write fast resume data
        std::vector<char> fastResumeData;
        libed2k::bencode(std::back_inserter(fastResumeData), *alert->resume_data);
        libed2k::transfer_resume_data trd(alert->m_handle.hash(), alert->m_handle.name(), alert->m_handle.size(), false,
                                          fastResumeData);
        std::string tcfg = alert->m_handle.save_path() + ".ex";
        std::ofstream fs(tcfg.c_str(), std::ios_base::out | std::ios_base::binary);
        if (fs) {
            libed2k::archive::ed2k_oarchive oa(fs);
            oa << trd;
            fs.close();
            DBG("ed2k_session_: resume data save success on" << tcfg);
        } else {
            ERR("ed2k_session_: resume data save fail with open error on" << tcfg);
        }
    } catch (std::exception& e) {
        ERR("ed2k_session_: resume data save fail with " << e.message() << " on" << tcfg);
    }
}
void ed2k_session_::on_resumed_data_transfer(libed2k::resumed_transfer_alert* alert) {
    DBG("ed2k_session_: on_resumed_data_transfer" << alert->m_handle.hash());
}
void ed2k_session_::on_paused_data_transfer(libed2k::paused_transfer_alert* alert) {
    DBG("ed2k_session_: on_paused_data_transfer" << alert->m_handle.hash());
}
void ed2k_session_::on_deleted_data_transfer(libed2k::deleted_transfer_alert* alert) {
    DBG("ed2k_session_: on_deleted_data_transfer" << alert->m_handle.hash());
}
void ed2k_session_::on_shutdown_completed() { DBG("ed2k_session_: shutdown completed"); }

void ed2k_session_::save_fast_resume(const boost::system::error_code& ec) {
    if (ec == boost::asio::error::operation_aborted) {
        return;
    }
    // actually we save nothing - only for test
    std::vector<libed2k::transfer_handle> v = ses->get_transfers();
    for (std::vector<libed2k::transfer_handle>::iterator i = v.begin(); i != v.end(); ++i) {
        libed2k::transfer_handle h = *i;
        if (!h.is_valid() || h.is_seed()) continue;
        try {
            if (h.status().state == libed2k::transfer_status::checking_files ||
                h.status().state == libed2k::transfer_status::queued_for_checking)
                continue;
            if (h.need_save_resume_data()) {
                h.save_resume_data();
            }
        } catch (libed2k::libed2k_exception& e) {
            ERR("save error: " << e.what());
        }
    }
    fs_timer.expires_at(fs_timer.expires_at() + fs_timer_delay);
    fs_timer.async_wait(boost::bind(&ed2k_session_::save_fast_resume, this, boost::asio::placeholders::error));
}
//
loader_::loader_() {}

void loader_::search_file(std::string hash, HashType type) {
    switch (type) {
        case MD4_H:
            ed2k_session_::search(hash);
            break;
        default:
            break;
    }
}

void loader_::search_file(std::string query, std::string extension, boost::uint64_t min_size,
                          boost::uint64_t max_size) {
    ed2k_session_::search(min_size, max_size, 0, 0, "", extension, "", 0, 0, query);
}
//
// namespace emulex
}
