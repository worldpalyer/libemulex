
#include "emulex/loader.hpp"
#include <fstream>

namespace emulex {

void db_interface_::load_settings(loader ld) {
    ld->ed2k.settings.peer_connect_timeout = 60;
    ld->ed2k.settings.peer_timeout = 60;
    ld->ed2k.settings.m_known_file = "./ws/known.met";
    ld->ed2k.settings.listen_port = 14668;
}

ed2k_session_::ed2k_session_() : alert_placeholder("", "", 10) {}

bool ed2k_session_::load_nodes(const std::string& filename) {
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

void ed2k_session_::start_kad(libed2k::session& ses, libed2k::kad_nodes_dat& knd) {
    for (size_t i = 0; i != knd.bootstrap_container.m_collection.size(); ++i) {
        DBG("bootstrap " << knd.bootstrap_container.m_collection[i].kid.toString()
                         << " ip:" << libed2k::int2ipstr(knd.bootstrap_container.m_collection[i].address.address)
                         << " udp:" << knd.bootstrap_container.m_collection[i].address.udp_port
                         << " tcp:" << knd.bootstrap_container.m_collection[i].address.tcp_port);
        ses.add_dht_node(std::make_pair(libed2k::int2ipstr(knd.bootstrap_container.m_collection[i].address.address),
                                        knd.bootstrap_container.m_collection[i].address.udp_port),
                         knd.bootstrap_container.m_collection[i].kid.toString());
    }
    for (std::list<libed2k::kad_entry>::const_iterator itr = knd.contacts_container.begin();
         itr != knd.contacts_container.end(); ++itr) {
        DBG("nodes " << itr->kid.toString() << " ip:" << libed2k::int2ipstr(itr->address.address)
                     << " udp:" << itr->address.udp_port << " tcp:" << itr->address.tcp_port);
        ses.add_dht_node(std::make_pair(libed2k::int2ipstr(itr->address.address), itr->address.udp_port),
                         itr->kid.toString());
    }
}

libed2k::session* ed2k_session_::create_session(const host& n) {
    libed2k::session* node = new libed2k::session(print, "0.0.0.0", settings);
    node->set_alert_mask(libed2k::alert::all_categories);
    node->set_alert_dispatch(boost::bind(&ed2k_session_::on_alert, this, boost::placeholders::_1));
    libed2k::server_connection_parameters scp(n.name, n.addr, n.port, 60, 60, 60, 60, 60);
    node->server_connect(scp);
    DBG("ED2K: connect server by " << n.addr << ":" << n.port);
    return node;
}

void ed2k_session_::init() {
    DBG("ED2K: start initial ed2k session by " << hosts.size() << " hosts");
    BOOST_FOREACH (const host& n, hosts) { ses.push_back(create_session(n)); }
    if (ses.size()) {
        start_kad(*ses[0], knd);
    }
}
void ed2k_session_::on_alert(libed2k::alert const& alert) {
    libed2k::alert* alert_ptr = (libed2k::alert*)&alert;
    if (libed2k::server_connection_initialized_alert* p =
            dynamic_cast<libed2k::server_connection_initialized_alert*>(alert_ptr)) {
        DBG("ALERT: server initalized: cid: " << p->client_id);
    } else if (libed2k::server_name_resolved_alert* p = dynamic_cast<libed2k::server_name_resolved_alert*>(alert_ptr)) {
        DBG("ALERT: server name was resolved: " << p->endpoint);
    } else if (libed2k::server_status_alert* p = dynamic_cast<libed2k::server_status_alert*>(alert_ptr)) {
        DBG("ALERT: server status: files count: " << p->files_count << " users count " << p->users_count);
    } else if (libed2k::server_message_alert* p = dynamic_cast<libed2k::server_message_alert*>(alert_ptr)) {
        DBG("ALERT: msg: " << p->server_message);
    } else if (libed2k::server_identity_alert* p = dynamic_cast<libed2k::server_identity_alert*>(alert_ptr)) {
        DBG("ALERT: server_identity_alert: " << p->server_hash << " name:  " << p->server_name
                                             << " descr: " << p->server_descr);
    } else if (libed2k::shared_files_alert* p = dynamic_cast<libed2k::shared_files_alert*>(alert_ptr)) {
        //        boost::mutex::scoped_lock l(m_sf_mutex);
        //        DBG("ALERT: RESULT: " << p->m_files.m_collection.size());
        //        vSF.clear();
        //        vSF = p->m_files;
        //
        //        boost::uint64_t nSize = 0;
        //
        //        for (size_t n = 0; n < vSF.m_size; ++n) {
        //            boost::shared_ptr<base_tag> low =
        //            vSF.m_collection[n].m_list.getTagByNameId(libed2k::FT_FILESIZE);
        //            boost::shared_ptr<base_tag> hi =
        //            vSF.m_collection[n].m_list.getTagByNameId(libed2k::FT_FILESIZE_HI);
        //            boost::shared_ptr<base_tag> src =
        //            vSF.m_collection[n].m_list.getTagByNameId(libed2k::FT_SOURCES);
        //
        //            if (low.get()) {
        //                nSize = low->asInt();
        //            }
        //
        //            if (hi.get()) {
        //                nSize += hi->asInt() << 32;
        //            }
        //
        //            DBG("ALERT: indx:" << n << " hash: " << vSF.m_collection[n].m_hFile.toString() << "name:"
        //                               << libed2k::convert_to_native(
        // vSF.m_collection[n].m_list.getStringTagByNameId(libed2k::FT_FILENAME))
        //                               << " size: " << nSize << " src: " << src->asInt());
        //            std::cout << "ALERT: indx:" << n << " hash: " << vSF.m_collection[n].m_hFile.toString()
        //                      << " name: " << libed2k::convert_to_native(
        // vSF.m_collection[n].m_list.getStringTagByNameId(libed2k::FT_FILENAME))
        //                      << " size: " << nSize << " src: " << src->asInt() << "\n";
        //        }
    } else if (libed2k::peer_message_alert* p = dynamic_cast<libed2k::peer_message_alert*>(alert_ptr)) {
        DBG("ALERT: MSG: ADDR: " << libed2k::int2ipstr(p->m_np.m_nIP) << " MSG " << p->m_strMessage);
    } else if (libed2k::peer_disconnected_alert* p = dynamic_cast<libed2k::peer_disconnected_alert*>(alert_ptr)) {
        DBG("ALERT: peer disconnected: " << libed2k::int2ipstr(p->m_np.m_nIP));
    } else if (libed2k::peer_captcha_request_alert* p = dynamic_cast<libed2k::peer_captcha_request_alert*>(alert_ptr)) {
        DBG("ALERT: captcha request ");
        FILE* fp = fopen("./captcha.bmp", "wb");
        if (fp) {
            fwrite(&p->m_captcha[0], 1, p->m_captcha.size(), fp);
            fclose(fp);
        }

    } else if (libed2k::peer_captcha_result_alert* p = dynamic_cast<libed2k::peer_captcha_result_alert*>(alert_ptr)) {
        DBG("ALERT: captcha result " << p->m_nResult);
    } else if (libed2k::peer_connected_alert* p = dynamic_cast<libed2k::peer_connected_alert*>(alert_ptr)) {
        DBG("ALERT: peer connected: " << libed2k::int2ipstr(p->m_np.m_nIP) << " status: " << p->m_active);
    } else if (libed2k::shared_files_access_denied* p = dynamic_cast<libed2k::shared_files_access_denied*>(alert_ptr)) {
        DBG("ALERT: peer denied access to shared files: " << libed2k::int2ipstr(p->m_np.m_nIP));
    } else if (libed2k::shared_directories_alert* p = dynamic_cast<libed2k::shared_directories_alert*>(alert_ptr)) {
        DBG("peer shared directories: " << libed2k::int2ipstr(p->m_np.m_nIP) << " count: " << p->m_dirs.size());
        for (size_t n = 0; n < p->m_dirs.size(); ++n) {
            DBG("ALERT: DIR: " << p->m_dirs[n]);
        }
    } else if (dynamic_cast<libed2k::save_resume_data_alert*>(alert_ptr)) {
        DBG("ALERT: save_resume_data_alert");
    } else if (dynamic_cast<libed2k::save_resume_data_failed_alert*>(alert_ptr)) {
        DBG("ALERT: save_resume_data_failed_alert");
    } else if (libed2k::transfer_params_alert* p = dynamic_cast<libed2k::transfer_params_alert*>(alert_ptr)) {
        if (!p->m_ec) {
            DBG("ALERT: transfer_params_alert, add transfer for: " << p->m_atp.file_path);
            // ps->add_transfer(p->m_atp);
        }
    } else {
        DBG("ALERT: Unknown alert: " << alert_ptr->message());
    }
}
//
loader_::loader_(boost::asio::io_service& ios, db_interface db) : ios(ios), db(db) {}

void loader_::init() {
    DBG("LOADER: start initial... ");
    db->load_settings(shared_from_this());
    ed2k.init();
}

//
// namespace emulex
}
