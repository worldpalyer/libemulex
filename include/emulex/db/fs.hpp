//
//  fs.hpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#ifndef fs_hpp
#define fs_hpp

#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sqlite3.h>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include "sqlite.hpp"
#include <vector>
#include "encoding.hpp"
#include "iconv.h"

namespace emulex {
namespace db {
#define ED2K_PART_L 9728000
#define EMULEX_FSDB_VER 1
#define EMULEX_TSDB_VER 1
//
#define EMULEX_FORMAT_L 8
//
enum HashType {
    EMD4 = 1,
    MD5 = 1 << 1,
    SHA1 = 1 << 2,
    ALL_HASH = EMD4 | MD5 | SHA1,
};

#define HASH_IS(type, T) (type & T) == T
/*
 the ed2k hash
 */
class Hash : public Data {
   public:
    HashType type;

   public:
    Hash();
    Hash(size_t len);
    Hash(const char *buf, size_t len);
    virtual ~Hash();
    virtual void set(HashType type);
    virtual void set(size_t len);
    virtual void set(const char *buf, size_t len);
    virtual bool operator==(const Hash &h) const;
    virtual bool operator<(const Hash &h) const;
    virtual std::string tostring();
};
Hash BuildHash(size_t len = 16);
Hash BuildHash(const char *buf, size_t len = 16);
Hash BuildHash(Data &data);
Hash FromHex(const char *hex);

std::string hash_tos(const char *hash);
typedef std::pair<uint32_t, uint32_t> Part;
class SortedPart : public std::vector<uint64_t> {
   public:
    uint64_t total;

   public:
    SortedPart(uint64_t total);
    virtual bool add(uint64_t av, uint64_t bv);
    virtual bool exists(uint64_t av, uint64_t bv);
    virtual bool isdone();
    virtual void print();
    virtual std::vector<uint8_t> parsePartStatus(size_t plen = ED2K_PART_L);
    virtual std::vector<Part> split(uint64_t max = 512000);
};

class FUUID_ {
   public:
    Hash sha1;      // 0x01
    Hash md5;       // 0x02
    Hash emd4;      // 0x03
    Data filename;  // 0x04
    Data location;
    uint64_t size = 0;  // 0x05

   public:
    virtual void cuuid(FUUID_ *v);
};
typedef boost::shared_ptr<FUUID_> FUUID;
FUUID BuildFUUID(const Hash &hash, HashType type);

struct FUUIDComparer {
    bool operator()(const FUUID &first, const FUUID &second) const;
};
//
#define FDSS_SHARING 100
#define FDSS_FILE 200
#define FDSS_DOWNLOADING 300
#define FDSD_VER 100
std::map<int, const char *> FS_VER_SQL();
class FData_ : public FUUID_ {
   public:
    uint64_t tid;
    Data format;       // 0x06
    double duration;   // 0x07
    double bitrate;    // 0x08
    Data codec;        // 0x09
    Data authors;      // 0x0A
    Data description;  // 0x0B
    Data album;        // 0x0C
    Data source;       // 0x0D
    int status = 0;
    // magic 0xEE
   public:
    void encode(Encoding &enc);
    void parse(Decoding &dec);
    uint16_t dsize();
};
typedef boost::shared_ptr<FData_> FData;
FData BuildFData(const char *spath);

class FDataDb_ : public SQLite_ {
   public:
    FDataDb_();
    virtual void init(const char *spath);
    virtual int count(int status = FDSS_SHARING);
    virtual std::vector<FData> list(int status = FDSS_SHARING, int skip = 0, int limit = 30);
    virtual uint64_t add(FData &file);
    virtual void remove(uint64_t tid);
    virtual FData find(Hash &hash, int type);
    virtual void updateFilename(uint64_t tid, Data filename);
};
typedef boost::shared_ptr<FDataDb_> FDataDb;
//
const int FTSS_RUNNING = 1;
const int FTSS_DONE = 1 << 1;
#define EX_DB_VER 100
std::map<int, const char *> EX_VER_SQL();
class FTask_ : public FUUID_ {
   public:
    uint64_t tid = 0;
    uint64_t done = 0;
    Data format;
    uint64_t used = 0;
    int status = 0;
};
typedef boost::shared_ptr<FTask_> FTask;

const int FSVS_ED2K = 1;
const int FSVS_EMULEX = 1 << 1;
const int FSVS_IRC = 1 << 2;

class FSrv_ {
   public:
    uint64_t tid = 0;
    Data name;
    Data addr;
    short port;
    int type;
    Data description;
    int tryc;
    uint64_t last;
};
typedef boost::shared_ptr<FSrv_> FSrv;
//
class EmuleX_ : public SQLite_ {
   public:
    EmuleX_();
    virtual void init(const char *spath);
    virtual int countTask(int status = FTSS_RUNNING);
    virtual std::vector<FTask> listTask(int status = FTSS_RUNNING | FTSS_DONE, int skip = 0, int limit = 30);
    virtual uint64_t addTask(FTask &task);
    virtual FTask addTask(const FUUID &uuid);
    virtual void removeTask(uint64_t tid);
    virtual void updateTask(uint64_t tid, int status);
    //
    virtual int countSrv();
    virtual std::vector<FSrv> listSrv(Data addr = Data(), short port = 0, int type = 0);
    virtual uint64_t addSrv(FSrv &srv);
    virtual FSrv addSrv(Data name, Data addr, short port, int type, Data description);
    virtual FSrv findSrv(Data addr, short port);
    virtual void removeSrv(uint64_t tid);
    virtual void updateSrv(uint64_t tid, int tryc, uint64_t last);
};
typedef boost::shared_ptr<EmuleX_> EmuleX;

#define write_data(file_, key_, data_)                                                        \
    file_.write(key_, 1)                                                                      \
        .write(endian::endian_buffer<endian::order::big, uint16_t, 16>(data_->len).data(), 2) \
        .write(data_->data, data_->len)
#define write_ui64(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint64_t, 64>(val_).data(), 8)
#define write_ui32(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint32_t, 32>(val_).data(), 4)
#define write_ui16(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint16_t, 16>(val_).data(), 2)
#define write_ui8(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint8_t, 8>(val_).data(), 1)
#define write_ui64_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint64_t, 64>(val_).data(), 8)
#define write_ui32_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint32_t, 32>(val_).data(), 4)
#define write_ui16_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint16_t, 16>(val_).data(), 2)
#define write_ui8_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint8_t, 8>(val_).data(), 1)

class FileConf_ : public FUUID_ {
   public:
    //    Data filename;           // 0x10
    //    uint64_t size;           // 0x20
    //    Hash emd4;               // 0x30
    std::vector<Hash> ed2k;  // 0x40
    //    Hash md5;                // 0x50
    //    Hash sha1;               // 0x60
    SortedPart parts;  // 0x70
   public:
    FileConf_(size_t size);
    virtual void save(const char *path);
    virtual void read(const char *path);
    virtual bool add(uint64_t av, uint64_t bv);
    virtual bool exists(size_t offset, size_t len);
    virtual bool isdone();
    virtual int readhash(const char *path, HashType type);
    virtual int readhash(std::fstream *file, HashType type);
    virtual std::vector<uint8_t> parsePartStatus(size_t plen = ED2K_PART_L);
    virtual std::vector<Part> split(uint64_t max = 512000);
};
typedef boost::shared_ptr<FileConf_> FileConf;
FileConf BuildFileConf(size_t size);
FileConf BuildFileConf(const char *path, HashType type = ALL_HASH);

//
class File_ {
   public:
    FileConf fc;
    std::fstream *fs;
    boost::filesystem::path spath;
    boost::filesystem::path tpath;
    boost::filesystem::path cpath;

   public:
    File_(const FUUID &file);
    //    File_(boost::filesystem::path dir, Data &filename);
    virtual ~File_();
    virtual bool exists(size_t av, size_t bv);
    virtual bool write(size_t offset, Data data);
    virtual bool write(size_t offset, const char *data, size_t len);
    virtual void read(size_t offset, Data data);
    virtual bool isdone();
    virtual std::vector<uint8_t> parsePartStatus(size_t plen = ED2K_PART_L);
    virtual std::vector<Part> split(uint64_t max = 512000);
    virtual bool valid(HashType type);
    virtual void close();
    virtual void done();
    virtual void clear();
};
typedef boost::shared_ptr<File_> File;

//
class FileManager_ {
   public:
    EmuleX ts;
    FDataDb fs;
    std::map<FUUID, File, FUUIDComparer> opened;

   public:
    FileManager_(const char *emulex, const char *fdb);
    File findOpenedF(const FUUID &uuid);
    File open(const FUUID &uuid);
    //    File open(boost::filesystem::path dir, Data &filename);
    virtual void done(const FUUID &uuid);
};
typedef boost::shared_ptr<FileManager_> FileManager;
//
void SetShared(FileManager fmgr);
FileManager &Shared();
}
}

#endif /* fs_hpp */
