//
//  fs.cpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#include "../../include/emulex/db/fs.hpp"

namespace emulex {
namespace db {

Hash::Hash() {}

Hash::Hash(size_t len) : Data(new Data_(len, false)) {}

Hash::Hash(const char *buf, size_t len) : Data(new Data_(buf, len)) {}

Hash::~Hash() {
    // V_LOG_FREE("%s", "Hash_ free...");
}

void Hash::set(size_t len) { this->reset(new Data_(len, false)); }

void Hash::set(HashType type) {
    switch (type) {
        case SHA1:
            set(SHA_DIGEST_LENGTH);
            break;
        case MD5:
            set(MD5_DIGEST_LENGTH);
            break;
        case EMD4:
            set(MD4_DIGEST_LENGTH);
            break;
        default:
            throw Fail("FoundSource parse fail with invalid hash type, emd4/md5/sha1 expected, but %x", type);
    }
    this->type = type;
}
void Hash::set(const char *buf, size_t len) { this->reset(new Data_(buf, len, false)); }

bool Hash::operator==(const Hash &h) const {
    Data_ *a = get();
    Data_ *b = h.get();
    if (a == 0 || b == 0) {
        return false;
    }
    if (a->len != b->len) {
        return false;
    }
    for (size_t i = 0; i < a->len; i++) {
        if (a->data[i] != b->data[i]) {
            return false;
        }
    }
    return true;
}

bool Hash::operator<(const Hash &h) const {
    Data_ *a = get();
    Data_ *b = h.get();
    if (a == 0 || b == 0) {
        return true;
    }
    if (a->len != b->len) {
        return true;
    }
    for (size_t i = 0; i < a->len; i++) {
        if (a->data[i] != b->data[i]) {
            return a->data[i] < b->data[i];
        }
    }
    return false;
}

std::string Hash::tostring() {
    Data_ *a = get();
    return hash_tos(a->data);
}

std::string hash_tos(const char *hash) {
    char buf[33];
    for (int i = 0; i < 16; i++) {
        sprintf(buf + i * 2, "%02x", (unsigned char)hash[i]);
    }
    return std::string(buf);
}

Hash BuildHash(size_t len) { return Hash(len); }

Hash BuildHash(const char *buf, size_t len) { return Hash(buf, len); }

Hash BuildHash(Data &data) { return Hash(data->data, data->len); }

Hash FromHex(const char *hex) {
    size_t len = strlen(hex);
    if (len < 2 || len % 2) {
        return Hash();
    }
    auto bys = BuildHash(len / 2);
    for (unsigned int i = 0; i < len; i += 2) {
        bys->data[i / 2] = hex2int(hex[i]) * 16 + hex2int(hex[i + 1]);
    }
    return bys;
}

SortedPart::SortedPart(uint64_t total) { this->total = total; }

bool SortedPart::add(uint64_t av, uint64_t bv) {
    size_t len = size() / 2;
    size_t i = 0;
    for (; i < len; i++) {
        if (av <= at(2 * i) + 1) {
            if (bv < at(2 * i)) {
                insert(begin() + 2 * i, bv);
                insert(begin() + 2 * i, av);
                return false;
            }
            if (bv <= at(2 * i + 1)) {
                (*this)[2 * i] = av;
                return false;
            }
            (*this)[2 * i] = av;
            (*this)[2 * i + 1] = bv;
            break;
        }
        if (av <= at(2 * i + 1) + 1) {
            if (bv <= at(2 * i + 1)) {
                return false;
            }
            (*this)[2 * i + 1] = bv;
            break;
        }
    }
    if (i == len) {
        insert(begin() + 2 * i, bv);
        insert(begin() + 2 * i, av);
        return isdone();
    }
    for (; i < len - 1;) {
        if (at(2 * i + 2) > at(2 * i + 1) + 1) {
            break;
        }
        if (at(2 * i + 1) >= at(2 * i + 3)) {
            erase(begin() + 2 * i + 3);
            erase(begin() + 2 * i + 2);
        } else {
            erase(begin() + 2 * i + 2);
            erase(begin() + 2 * i + 1);
        }
        len--;
    }
    return isdone();
}

bool SortedPart::exists(uint64_t av, uint64_t bv) {
    size_t pl = size() / 2;
    for (size_t i = 0; i < pl; i++) {
        if (bv <= at(2 * i + 1)) {
            if (av >= at(2 * i)) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

bool SortedPart::isdone() { return size() == 2 && at(0) == 0 && at(1) == total - 1; }

void SortedPart::print() {
    size_t len = size();
    for (size_t i = 0; i < len; i++) {
        printf("%llu ", at(i));
    }
    printf("\n");
}

std::vector<uint8_t> SortedPart::parsePartStatus(size_t plen) {
    std::vector<uint8_t> parts;
    size_t len = size() / 2;
    uint8_t part = 0;
    long pidx = 0;
    int dc = 0;
    for (size_t i = 0; i < len; i++) {
        long offset = at(2 * i) / plen;
        if (at(2 * i) % plen > 0) {
            offset++;
        }
        dc += offset - pidx;
        while (dc > 8) {
            parts.push_back(part);
            dc -= 8;
            part = 0;
        }
        pidx = offset;
        //
        long ac = (at(2 * i + 1) - at(2 * i) + 1) / plen;
        for (long j = 0; j < ac; j++) {
            part = (1 << (7 - dc)) | part;
            dc++;
            if (dc < 8) {
                continue;
            }
            parts.push_back(part);
            dc = 0;
            part = 0;
        }
        pidx = offset + ac;
    }
    if (dc > 0) {
        parts.push_back(part);
    }
    return parts;
}

std::vector<Part> SortedPart::split(uint64_t max) {
    std::vector<Part> ps;
    size_t slen = total / max;
    for (size_t i = 0; i < slen; i++) {
        if (exists(i * max, i * max + max - 1)) {
            continue;
        } else {
            ps.push_back(Part(i * max, i * max + max));
        }
    }
    if (slen * max < total - 1 && !exists(slen * max, total - 1)) {
        ps.push_back(Part(slen * max, total));
    }
    return ps;
}

void FUUID_::cuuid(FUUID_ *v) {
    this->sha1 = v->sha1;
    this->md5 = v->md5;
    this->emd4 = v->emd4;
    this->filename = v->filename;
    this->location = v->location;
    this->size = v->size;
}

FUUID BuildFUUID(const Hash &hash, HashType type) {
    auto uuid = FUUID(new FUUID_);
    switch (type) {
        case EMD4:
            uuid->emd4 = hash;
            break;
        case MD5:
            uuid->md5 = hash;
            break;
        case SHA1:
            uuid->sha1 = hash;
            break;
        default:
            throw Fail("Build FUUID fail with %s", "the type must in emd4/md5/sha1");
    }
    return uuid;
}

bool FUUIDComparer::operator()(const FUUID &first, const FUUID &second) const {
    if (first->sha1.get() && second->sha1.get()) {
        return DataComparer()(first->sha1, second->sha1);
    }
    if (first->md5.get() && second->md5.get()) {
        return DataComparer()(first->md5, second->md5);
    }
    if (first->emd4.get() && second->emd4.get()) {
        return DataComparer()(first->emd4, second->emd4);
    }
    if (first->location.get() && second->location.get() && !first->location->cmp(second->location)) {
        return DataComparer()(first->location, second->location);
    }
    if (first->filename.get() && second->filename.get()) {
        return DataComparer()(first->filename, second->filename);
    }
    return false;
}

void FData_::encode(Encoding &enc) {
    if (sha1.get()) {
        enc.put((uint8_t)0x01);
        enc.put(sha1);
    }
    if (md5.get()) {
        enc.put((uint8_t)0x02);
        enc.put(md5);
    }
    if (emd4.get()) {
        enc.put((uint8_t)0x03);
        enc.put(emd4);
    }
    if (filename.get()) {
        enc.put((uint8_t)0x04);
        enc.put((uint16_t)filename->len);
        enc.put(filename);
    }
    if (size) {
        enc.put((uint8_t)0x05);
        enc.put(size);
    }
    if (format.get()) {
        enc.put((uint8_t)0x06);
        enc.put((uint16_t)format->len);
        enc.put(format);
    }
    if (description.get()) {
        enc.put((uint8_t)0x0B);
        enc.put((uint16_t)description->len);
        enc.put(description);
    }
    if (source.get()) {
        enc.put((uint8_t)0x0D);
        enc.put((uint16_t)source->len);
        enc.put(source);
    }
    enc.put((uint8_t)0xEE);
}

void FData_::parse(Decoding &dec) {
    uint8_t magic;
    magic = dec.get<uint8_t, 1>();
    if (magic != 0xEE) {
        throw Fail("FData parse fail with invalid magic, %x expected, but %x", 0xEE, magic);
    }
    while (true) {
        magic = dec.get<uint8_t, 1>();
        switch (magic) {
            case 0x01: {
                sha1.set(SHA_DIGEST_LENGTH);
                dec.get(sha1->data, sha1->len);
                break;
            }
            case 0x02: {
                md5.set(MD5_DIGEST_LENGTH);
                dec.get(md5->data, md5->len);
                break;
            }
            case 0x03: {
                emd4.set(MD4_DIGEST_LENGTH);
                dec.get(emd4->data, emd4->len);
                break;
            }
            case 0x04: {
                filename.reset(new Data_(dec.get<uint16_t, 2>(), true));
                dec.get(filename->data, filename->len);
                break;
            }
            case 0x05: {
                size = dec.get<uint64_t, 8>();
                break;
            }
            case 0x06: {
                format.reset(new Data_(dec.get<uint16_t, 2>(), true));
                dec.get(format->data, format->len);
                break;
            }
            case 0x0B: {
                description.reset(new Data_(dec.get<uint16_t, 2>(), true));
                dec.get(description->data, description->len);
                break;
            }
            case 0x0D: {
                source.reset(new Data_(dec.get<uint16_t, 2>(), true));
                dec.get(source->data, source->len);
                break;
            }
            case 0xEE: {
                return;
            }
            default: { throw Fail("FData parse fail with invalid item magic %x ", magic); }
        }
    }
}

uint16_t FData_::dsize() {
    uint16_t size = 0;
    if (sha1.get()) {
        size += SHA_DIGEST_LENGTH + 1;
    }
    if (md5.get()) {
        size += MD5_DIGEST_LENGTH + 1;
    }
    if (emd4.get()) {
        size += MD4_DIGEST_LENGTH + 1;
    }
    if (filename.get()) {
        size += filename->len + 3;
    }
    if (size) {
        size += 9;
    }
    if (format.get()) {
        size += format->len + 3;
    }
    if (description.get()) {
        size += description->len + 3;
    }
    if (source.get()) {
        size += source->len + 3;
    }
    size += 1;
    return size;
}

FData BuildFData(const char *spath) {
    auto fc = BuildFileConf(spath, ALL_HASH);
    auto fd = FData(new FData_());
    fd->cuuid(fc.get());
    auto fpath = boost::filesystem::path(spath);
    if (fpath.has_extension()) {
        auto ext = fpath.extension();
        auto extc = ext.c_str();
        fd->format = BuildData(extc, strlen(extc));
    }
    fd->location = BuildData(spath, strlen(spath));
    fd->status = FDSS_SHARING;
    return fd;
}

FDataDb_::FDataDb_() : SQLite_(FDSD_VER) {}

void FDataDb_::init(const char *spath) { SQLite_::init(spath, FS_VER_SQL()); }

int FDataDb_::count(int status) { return intv("select count(*) from ex_file where status=%d", status); }

std::vector<FData> parseFDataStmt(STMT stmt) {
    std::vector<FData> fs;
    while (stmt->step()) {
        int idx = 0;
        auto file = FData(new FData_);
        file->tid = stmt->intv(idx++);
        auto sha1 = stmt->blobv(idx++);
        file->sha1 = BuildHash(sha1);
        auto md5 = stmt->blobv(idx++);
        ;
        file->md5 = BuildHash(md5);
        auto emd4 = stmt->blobv(idx++);
        file->emd4 = BuildHash(emd4);
        file->filename = stmt->stringv(idx++);
        file->size = stmt->intv(idx++);
        file->format = stmt->stringv(idx++);
        file->location = stmt->stringv(idx++);
        file->duration = stmt->floatv(idx++);
        file->bitrate = stmt->floatv(idx++);
        file->codec = stmt->stringv(idx++);
        file->authors = stmt->stringv(idx++);
        file->description = stmt->stringv(idx++);
        file->album = stmt->stringv(idx++);
        file->status = (int)stmt->intv(idx++);
        fs.push_back(file);
    }
    return fs;
}

std::vector<FData> FDataDb_::list(int status, int skip, int limit) {
    auto stmt = prepare(
        "select tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,status "
        " from ex_file where status=%d limit %d,%d",
        status, skip, limit);
    return parseFDataStmt(stmt);
}

uint64_t FDataDb_::add(FData &task) {
    auto stmt = prepare(
        "insert into ex_file ("
        "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,status"
        ") values ("
        "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?"
        ") ");
    int idx = 1;
    stmt->bind(idx++);
    if (task->sha1.get()) {
        stmt->bind(idx++, task->sha1);
    } else {
        stmt->bind(idx++);
    }
    if (task->md5.get()) {
        stmt->bind(idx++, task->md5);
    } else {
        stmt->bind(idx++);
    }
    if (task->emd4.get()) {
        stmt->bind(idx++, task->emd4);
    } else {
        stmt->bind(idx++);
    }
    stmt->bind(idx++, task->filename);
    stmt->bind(idx++, (sqlite3_int64)task->size);
    if (task->format.get()) {
        stmt->bind(idx++, task->format);
    } else {
        stmt->bind(idx++);
    }
    if (task->location.get()) {
        stmt->bind(idx++, task->location);
    } else {
        stmt->bind(idx++);
    }
    stmt->bind(idx++, task->duration);
    stmt->bind(idx++, task->bitrate);
    if (task->codec.get()) {
        stmt->bind(idx++, task->codec);
    } else {
        stmt->bind(idx++);
    }
    if (task->authors.get()) {
        stmt->bind(idx++, task->authors);
    } else {
        stmt->bind(idx++);
    }
    if (task->description.get()) {
        stmt->bind(idx++, task->description);
    } else {
        stmt->bind(idx++);
    }
    if (task->album.get()) {
        stmt->bind(idx++, task->album);
    } else {
        stmt->bind(idx++);
    }
    stmt->bind(idx++, task->status);
    stmt->step();
    stmt->finalize();
    return SQLite_::intv("select last_insert_rowid()");
}

void FDataDb_::remove(uint64_t tid) { SQLite_::exec("delete from ex_file where tid=%d", tid); }

FData FDataDb_::find(Hash &hash, int type) {
    STMT stmt;
    switch (type) {
        case 0:
            stmt = prepare(
                "select "
                "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,"
                "status "
                " from ex_file where sha=?");
            break;
        case 1:
            stmt = prepare(
                "select "
                "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,"
                "status "
                " from ex_file where md5=?");
            break;
        case 2:
            stmt = prepare(
                "select "
                "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,"
                "status "
                " from ex_file where emd4=?");
            break;
        default:
            throw Fail("FDataDb_ find unknow type(%d)", type);
            break;
    }
    stmt->bind(1, hash);
    auto fs = parseFDataStmt(stmt);
    if (fs.size()) {
        return fs[0];
    } else {
        return FData();
    }
}

void FDataDb_::updateFilename(uint64_t tid, Data filename) {
    auto stmt = prepare("update ex_file set filename=? where tid=?");
    stmt->bind(1, filename);
    stmt->bind(2, (sqlite3_int64)tid);
    stmt->step();
}

EmuleX_::EmuleX_() : SQLite_(EX_DB_VER) {}

void EmuleX_::init(const char *spath) { SQLite_::init(spath, EX_VER_SQL()); }

int EmuleX_::countTask(int status) { return intv("select count(*) from ex_task where status=%d", status); }

std::vector<FTask> EmuleX_::listTask(int status, int skip, int limit) {
    std::vector<FTask> ts;
    auto stmt = prepare(
        "select tid,filename,location,size,done,format,used,status from ex_task where status=status&%d limit %d,%d",
        status, skip, limit);
    while (stmt->step()) {
        int idx = 0;
        auto task = FTask(new FTask_);
        task->tid = stmt->intv(idx++);
        task->filename = stmt->stringv(idx++);
        task->location = stmt->stringv(idx++);
        task->size = stmt->intv(idx++);
        task->done = stmt->intv(idx++);
        task->format = stmt->stringv(idx++);
        task->used = stmt->intv(idx++);
        task->status = (int)stmt->intv(idx++);
        ts.push_back(task);
    }
    return ts;
}

uint64_t EmuleX_::addTask(FTask &task) {
    auto format = "";
    if (task->format.get()) {
        format = task->format->data;
    }
    SQLite_::exec(
        "insert into ex_task (tid,filename,location,size,done,format,used,status) values "
        "(null,'%s','%s',%lu,%lu,'%s',%lu,%d)",
        task->filename->data, task->location->data, task->size, task->done, format, task->used, task->status);
    auto tid = SQLite_::intv("select last_insert_rowid()");
    task->tid = tid;
    return tid;
}

FTask EmuleX_::addTask(const FUUID &uuid) {
    auto task = FTask(new FTask_);
    task->cuuid(uuid.get());
    boost::filesystem::path filename(uuid->filename->data);
    if (filename.has_extension()) {
        auto extc = filename.extension().c_str();
        task->format = BuildData(extc, strlen(extc));
    }
    task->status = FTSS_RUNNING;
    addTask(task);
    return task;
}

void EmuleX_::removeTask(uint64_t tid) { SQLite_::exec("delete from ex_task where tid=%d", tid); }

void EmuleX_::updateTask(uint64_t tid, int status) {
    SQLite_::exec("update ex_task set status=%d where tid=%lu", status, tid);
}

int EmuleX_::countSrv() { return intv("select count(*) from ex_server"); }

std::vector<FSrv> EmuleX_::listSrv(Data addr, short port, int type) {
    std::vector<FSrv> ss;
    STMT stmt;
    if (addr.get()) {
        stmt = prepare(
            "select tid,name,addr,port,type,description,tryc,last from ex_server where addr='%s' and port=%d and "
            "type=type&%d",
            addr->data, port, type);
    } else {
        stmt = prepare("select tid,name,addr,port,type,description,tryc,last from ex_server where type=type&%d", type);
    }
    while (stmt->step()) {
        int idx = 0;
        auto srv = FSrv(new FSrv_);
        srv->tid = stmt->intv(idx++);
        srv->name = stmt->stringv(idx++);
        srv->addr = stmt->stringv(idx++);
        srv->port = (short)stmt->intv(idx++);
        srv->type = (int)stmt->intv(idx++);
        srv->description = stmt->stringv(idx++);
        srv->tryc = stmt->intv(idx++);
        srv->last = stmt->intv(idx++);
        ss.push_back(srv);
    }
    return ss;
}

uint64_t EmuleX_::addSrv(FSrv &srv) {
    const char *name = "";
    if (srv->name.get()) {
        name = srv->name->data;
    }
    const char *desc;
    if (srv->description.get()) {
        desc = srv->description->data;
    }
    SQLite_::exec(
        "insert into ex_server (tid,name,addr,port,type,description,tryc,last) values "
        "(null,'%s','%s',%lu,%lu,'%s',%lu,%d)",
        name, srv->addr->data, srv->port, srv->type, desc, srv->tryc, srv->last);
    auto tid = SQLite_::intv("select last_insert_rowid()");
    srv->tid = tid;
    return tid;
}
FSrv EmuleX_::addSrv(Data name, Data addr, short port, int type, Data description) {
    FSrv srv = FSrv(new FSrv_);
    srv->name = name;
    srv->addr = addr;
    srv->port = port;
    srv->type = type;
    srv->description = description;
    addSrv(srv);
    return srv;
}
FSrv EmuleX_::findSrv(Data addr, short port) {
    auto ss = listSrv(addr, port, 0);
    if (ss.size()) {
        return ss[0];
    } else {
        return FSrv();
    }
}
void EmuleX_::removeSrv(uint64_t tid) { SQLite_::exec("delete from ex_server where tid=%d", tid); }
void EmuleX_::updateSrv(uint64_t tid, int tryc, uint64_t last) {
    SQLite_::exec("update ex_server set tryc=%d,last=%lu where tid=%lu", tryc, last, tid);
}

FileConf_::FileConf_(size_t size) : parts(size) {}

void FileConf_::save(const char *path) {
    Encoding enc;
    enc.put((uint8_t)0x10).put((uint16_t)filename->len).put(filename);
    enc.put((uint8_t)0x20).put((uint64_t)size);
    if (emd4.get()) {
        enc.put((uint8_t)0x30).put((uint16_t)emd4->len).put(emd4);
    }
    if (ed2k.size()) {
        enc.put((uint8_t)0x40).put((uint16_t)ed2k.size());
        BOOST_FOREACH (Hash &h, ed2k) {
            enc.put((uint16_t)h->len);
            enc.put(h);
        }
    }
    if (md5.get()) {
        enc.put((uint8_t)0x50).put((uint16_t)md5->len).put(md5);
    }
    if (sha1.get()) {
        enc.put((uint8_t)0x60).put((uint16_t)sha1->len).put(sha1);
    }
    if (parts.size()) {
        enc.put((uint8_t)0x70).put((uint16_t)parts.size());
        BOOST_FOREACH (uint64_t &v, parts) { enc.put(v); }
    }
    std::fstream file;
    file.open(path, std::fstream::out | std::fstream::trunc);
    file.write(enc.cbuf(), enc.size());
    file.close();
}

void FileConf_::read(const char *path) {
    std::fstream file;
    file.open(path, std::fstream::in | std::fstream::ate);
    if (!file.is_open()) {
        throw Fail("FileConf_ open path(%s) fail", path);
    }
    Data data = BuildData(file.tellg());
    file.seekg(0);
    file.read(data->data, data->len);
    file.close();
    Decoding dec(data);
    while (dec.offset < data->len) {
        switch (dec.get<uint8_t, 1>()) {
            case 0x10: {
                filename = BuildData(dec.get<uint16_t, 2>(), true);
                dec.get(filename->data, filename->len);
                break;
            }
            case 0x20: {
                size = dec.get<uint64_t, 8>();
                parts.total = size;
                break;
            }
            case 0x30: {
                emd4 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(emd4->data, emd4->len);
                break;
            }
            case 0x40: {
                ed2k.clear();
                uint16_t pc = dec.get<uint16_t, 2>();
                for (size_t i = 0; i < pc; i++) {
                    Hash h = BuildHash(dec.get<uint16_t, 2>());
                    dec.get(h->data, h->len);
                    ed2k.push_back(h);
                }
                break;
            }
            case 0x50: {
                md5 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(md5->data, md5->len);
                break;
            }
            case 0x60: {
                sha1 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(sha1->data, sha1->len);
                break;
            }
            case 0x70: {
                parts.clear();
                uint16_t pc = dec.get<uint16_t, 2>();
                for (size_t i = 0; i < pc; i++) {
                    parts.push_back(dec.get<uint64_t, 8>());
                }
                break;
            }
        }
    }
}

bool FileConf_::add(uint64_t av, uint64_t bv) { return parts.add(av, bv); }

bool FileConf_::exists(size_t offset, size_t len) { return parts.exists(offset, len); }

bool FileConf_::isdone() { return parts.isdone(); }

int FileConf_::readhash(const char *path, HashType type) {
    std::fstream file;
    try {
        file.open(path, std::fstream::in | std::fstream::ate);
        if (!file.is_open()) {
            file.close();
            throw LFail(strlen(path) + 64, "FileConf_ read hash open path(%s) fail", path);
        }
        auto fname = boost::filesystem::path(path).filename();
        auto fnamec = fname.c_str();
        filename = BuildData(fnamec, strlen(fnamec));
        auto res = readhash(&file, type);
        file.close();
        return res;
    } catch (...) {
        file.close();
        throw LFail(strlen(path) + 64, "FileConf_ read hash path(%s) fail", path);
    }
}
int FileConf_::readhash(std::fstream *file, HashType type) {
    MD4_CTX fmd4;
    if (HASH_IS(type, EMD4)) {
        MD4_Init(&fmd4);
    }
    MD5_CTX fmd5;
    if (HASH_IS(type, MD5)) {
        MD5_Init(&fmd5);
    }
    SHA_CTX fsha;
    if (HASH_IS(type, SHA1)) {
        SHA1_Init(&fsha);
    }
    size = file->tellg();
    parts.total = size;
    file->seekg(0);
    file->seekp(0);
    char buf[9728];
    size_t readed = 0;
    unsigned char digest[MD4_DIGEST_LENGTH];
    while (!file->eof()) {
        MD4_CTX pmd4;
        if (HASH_IS(type, EMD4)) {
            MD4_Init(&pmd4);
        }
        for (int i = 0; i < 1000 && !file->eof(); i++) {
            file->read(buf, 9728);
            readed = file->gcount();
            if (HASH_IS(type, EMD4)) {
                MD4_Update(&pmd4, buf, readed);
            }
            if (HASH_IS(type, MD5)) {
                MD5_Update(&fmd5, buf, readed);
            }
            if (HASH_IS(type, SHA1)) {
                SHA1_Update(&fsha, buf, readed);
            }
        }
        if (HASH_IS(type, EMD4)) {
            MD4_Final(digest, &pmd4);
            ed2k.push_back(BuildHash((const char *)digest, MD4_DIGEST_LENGTH));
            MD4_Update(&fmd4, digest, MD4_DIGEST_LENGTH);
        }
    }
    if (HASH_IS(type, EMD4)) {
        emd4.set(MD4_DIGEST_LENGTH);
        MD4_Final((unsigned char *)emd4->data, &fmd4);
        if (ed2k.size() == 1) {
            emd4 = ed2k[0];
        }
    }
    if (HASH_IS(type, MD5)) {
        md5.set(MD5_DIGEST_LENGTH);
        MD5_Final((unsigned char *)md5->data, &fmd5);
    }
    if (HASH_IS(type, SHA1)) {
        sha1.set(SHA_DIGEST_LENGTH);
        SHA1_Final((unsigned char *)sha1->data, &fsha);
    }
    return 0;
}

std::vector<uint8_t> FileConf_::parsePartStatus(size_t plen) { return parts.parsePartStatus(plen); }

std::vector<Part> FileConf_::split(uint64_t max) { return parts.split(max); }

FileConf BuildFileConf(size_t size) { return FileConf(new FileConf_(size)); }

FileConf BuildFileConf(const char *path, HashType type) {
    auto fc = FileConf(new FileConf_(0));
    fc->readhash(path, type);
    return fc;
}

File_::File_(const FUUID &uuid) {
    this->fc = FileConf(new FileConf_(uuid->size));
    this->fc->cuuid(uuid.get());
    this->spath = boost::filesystem::path(uuid->location->data).append(uuid->filename->data);
    this->tpath = boost::filesystem::path(spath.string() + ".xdm");
    this->cpath = boost::filesystem::path(spath.string() + ".xcm");
    this->fc->save(cpath.c_str());
    this->fs = new std::fstream();
    this->fs->open(tpath.c_str(), std::fstream::out | std::fstream::binary);
    if (!this->fs->is_open()) {
        throw Fail("File_ open file(%s) fail", tpath.c_str());
    }
    if (this->fs->tellg() < 1) {
        fs->seekp(this->fc->size - 1);
        fs->write("\0", 1);
    }
}

// File_::File_(boost::filesystem::path dir, Data &filename) {
//    this->fc = FileConf(new FileConf_(0));
//    this->spath = dir.append(filename->data);
//    this->tpath = boost::filesystem::path(spath.string() + ".xdm");
//    this->cpath = boost::filesystem::path(spath.string() + ".xcm");
//    this->fc->read(this->cpath.c_str());
//    this->fs = new std::fstream();
//    this->fs->open(tpath.c_str(), std::fstream::out | std::fstream::binary);
//    if (!this->fs->is_open()) {
//        throw Fail("File_ open file(%s) fail", tpath.c_str());
//    }
//    if (this->fs->tellg() < 1) {
//        fs->seekp(this->fc->size - 1);
//        fs->write("\0", 1);
//    }
//}

File_::~File_() { close(); }

bool File_::exists(size_t av, size_t bv) { return fc->exists(av, bv); }

bool File_::write(size_t offset, Data data) { return write(offset, data->data, data->len); }

bool File_::write(size_t offset, const char *data, size_t len) {
    fs->seekp(offset);
    fs->write(data, len);
    bool done = fc->add(offset, offset + len - 1);
    fc->save(cpath.c_str());
    return done;
}

void File_::read(size_t offset, Data data) {
    fs->seekg(offset);
    fs->read(data->data, data->len);
}

bool File_::isdone() { return fc->isdone(); }

std::vector<uint8_t> File_::parsePartStatus(size_t plen) { return fc->parsePartStatus(plen); }

std::vector<Part> File_::split(uint64_t max) { return fc->split(max); }

bool File_::valid(HashType type) {
    fs->flush();
    auto cfc = FileConf(new FileConf_(0));
    cfc->readhash(tpath.c_str(), type);
    if (HASH_IS(type, EMD4)) {
        return cfc->emd4->cmp(fc->emd4);
    }
    if (HASH_IS(type, MD5)) {
        return cfc->md5->cmp(fc->md5);
    }
    if (HASH_IS(type, SHA1)) {
        return cfc->sha1->cmp(fc->sha1);
    }
    return false;
}

void File_::close() {
    if (fs) {
        fs->close();
        fs = 0;
    }
}

void File_::done() {
    close();
    boost::filesystem::rename(tpath, spath);
    boost::filesystem::remove_all(cpath);
}

void File_::clear() {
    close();
    boost::filesystem::remove_all(tpath);
    boost::filesystem::remove_all(cpath);
}

FileManager_::FileManager_(const char *emulex, const char *fdb) {
    ts = EmuleX(new EmuleX_);
    fs = FDataDb(new FDataDb_);
    ts->init(emulex);
    fs->init(fdb);
}

File FileManager_::findOpenedF(const FUUID &uuid) { return opened.at(uuid); }

File FileManager_::open(const FUUID &uuid) {
    if (opened.find(uuid) == opened.end()) {
        auto f = File(new File_(uuid));
        opened[uuid] = f;
        return f;
    } else {
        return opened[uuid];
    }
}

// File FileManager_::open(boost::filesystem::path dir, Data &filename) {
//    auto spath=dir.append(filename->data);
//    if(sopened.find(spath)!=sopened.end()){
//        return sopened[spath];
//    }
//    auto f = File(new File_(dir, filename));
//    if (f->fc->sha1.get()) {
//        opened[f->fc->sha1] = f;
//    }
//    if (f->fc->md5.get()) {
//        opened[f->fc->md5] = f;
//    }
//    if (f->fc->emd4.get()) {
//        opened[f->fc->emd4] = f;
//    }
//    return f;
//}

void FileManager_::done(const FUUID &uuid) {
    auto file = findOpenedF(uuid);
    if (file.get()) {
        file->done();
        opened.erase(uuid);
    }
}
//
FileManager fshared;
void SetShared(FileManager fmgr) { fshared = fmgr; }
FileManager &Shared() { return fshared; }
//
}
}
