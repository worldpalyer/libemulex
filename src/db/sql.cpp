//
//  sql.cpp
//  emule-x
//
//  Created by Centny on 1/19/17.
//
//
#include <map>
#include <string>

namespace emulex {
namespace db {
#define FS_SQL_V0 \
    "\
/*==============================================================*/\
/* Table: ex_file                                               */\
/*==============================================================*/\
create table ex_file\
(\
   tid                  integer primary key autoincrement,\
   sha                  blob,\
   md5                  blob,\
   emd4                 blob,\
   filename             varchar(256) not null,\
   size                 int not null,\
   format               varchar(16),\
   location             varchar(10240),\
   duration             numeric,\
   bitrate              numeric,\
   codec                varchar(32),\
   authors              varchar(256),\
   description          varchar(2048),\
   album                varchar(256),\
   status               int not null\
);\
\
/*==============================================================*/\
/* Index: ex_f_sha                                              */\
/*==============================================================*/\
create unique index ex_f_sha on ex_file\
(\
   sha\
);\
\
/*==============================================================*/\
/* Index: ex_f_md5                                              */\
/*==============================================================*/\
create unique index ex_f_md5 on ex_file\
(\
   md5\
);\
\
/*==============================================================*/\
/* Index: ex_f_emd4                                             */\
/*==============================================================*/\
create unique index ex_f_emd4 on ex_file\
(\
   emd4\
);\
\
/*==============================================================*/\
/* Index: ex_f_filename                                         */\
/*==============================================================*/\
create index ex_f_filename on ex_file\
(\
   filename\
);\
\
/*==============================================================*/\
/* Index: ex_f_size                                             */\
/*==============================================================*/\
create index ex_f_size on ex_file\
(\
   size\
);\
\
/*==============================================================*/\
/* Index: ex_f_format                                           */\
/*==============================================================*/\
create index ex_f_format on ex_file\
(\
   format\
);\
\
/*==============================================================*/\
/* Index: ex_f_location                                         */\
/*==============================================================*/\
create index ex_f_location on ex_file\
(\
   location\
);\
\
/*==============================================================*/\
/* Index: ex_f_duration                                         */\
/*==============================================================*/\
create index ex_f_duration on ex_file\
(\
   duration\
);\
\
/*==============================================================*/\
/* Index: ex_f_bitrate                                          */\
/*==============================================================*/\
create index ex_f_bitrate on ex_file\
(\
   bitrate\
);\
\
/*==============================================================*/\
/* Index: ex_f_codec                                            */\
/*==============================================================*/\
create index ex_f_codec on ex_file\
(\
   codec\
);\
\
/*==============================================================*/\
/* Index: ex_f_authors                                          */\
/*==============================================================*/\
create index ex_f_authors on ex_file\
(\
   authors\
);\
\
/*==============================================================*/\
/* Index: ex_f_desc                                             */\
/*==============================================================*/\
create index ex_f_desc on ex_file\
(\
   description\
);\
\
/*==============================================================*/\
/* Index: ex_f_album                                            */\
/*==============================================================*/\
create index ex_f_album on ex_file\
(\
   album\
);\
\
/*==============================================================*/\
/* Index: ex_f_status                                           */\
/*==============================================================*/\
create index ex_f_status on ex_file\
(\
   status\
);\
"

#define TS_SQL_V0 \
    "\
/*==============================================================*/\
/* Table: ex_env                                                */\
/*==============================================================*/\
create table ex_env\
(\
   name                 varchar(32) not null,\
   vala                 varchar(10240),\
   valb                 varchar(10240),\
   valc                 varchar(10240),\
   vald                 varchar(10240),\
   primary key (name)\
);\
\
\
/*==============================================================*/\
/* Table: ex_task                                               */\
/*==============================================================*/\
create table ex_task\
(\
   tid                  integer primary key autoincrement,\
   filename             varchar(256) not null,\
   location             varchar(4096) not null,\
   size                 int not null,\
   done                 int not null,\
   format               varchar(8),\
   used                 int not null,\
   status               int not null\
);\
\
/*==============================================================*/\
/* Index: ex_t_filename                                         */\
/*==============================================================*/\
create index ex_t_filename on ex_task\
(\
   filename\
);\
\
/*==============================================================*/\
/* Index: ex_t_location                                         */\
/*==============================================================*/\
create index ex_t_location on ex_task\
(\
   location\
);\
\
/*==============================================================*/\
/* Index: ex_t_size                                             */\
/*==============================================================*/\
create index ex_t_size on ex_task\
(\
   size\
);\
\
/*==============================================================*/\
/* Index: ex_t_done                                             */\
/*==============================================================*/\
create index ex_t_done on ex_task\
(\
   done\
);\
\
/*==============================================================*/\
/* Index: ex_t_format                                           */\
/*==============================================================*/\
create index ex_t_format on ex_task\
(\
   format\
);\
\
/*==============================================================*/\
/* Index: ex_t_used                                             */\
/*==============================================================*/\
create index ex_t_used on ex_task\
(\
   used\
);\
\
/*==============================================================*/\
/* Index: ex_t_status                                           */\
/*==============================================================*/\
create index ex_t_status on ex_task\
(\
   status\
);\
/*==============================================================*/\
/* Table: ex_server                                             */\
/*==============================================================*/\
create table ex_server\
(\
   tid                  integer primary key autoincrement,\
   name                 varchar(256),\
   addr                 varchar(256) not null,\
   port                 int not null,\
   type                 int not null,\
   description          varchar(1024),\
   tryc                 int,\
   last                 int\
);\
\
/*==============================================================*/\
/* Index: ex_s_name                                             */\
/*==============================================================*/\
create index ex_s_name on ex_server\
(\
   name\
);\
\
/*==============================================================*/\
/* Index: ex_s_addr                                             */\
/*==============================================================*/\
create index ex_s_addr on ex_server\
(\
   addr\
);\
\
/*==============================================================*/\
/* Index: ex_s_port                                             */\
/*==============================================================*/\
create index ex_s_port on ex_server\
(\
   port\
);\
\
/*==============================================================*/\
/* Index: ex_s_type                                             */\
/*==============================================================*/\
create index ex_s_type on ex_server\
(\
   type\
);\
\
/*==============================================================*/\
/* Index: ex_s_description                                      */\
/*==============================================================*/\
create index ex_s_description on ex_server\
(\
   description\
);\
\
/*==============================================================*/\
/* Index: ex_s_tryc                                             */\
/*==============================================================*/\
create index ex_s_tryc on ex_server\
(\
   tryc\
);\
\
/*==============================================================*/\
/* Index: ex_s_last                                             */\
/*==============================================================*/\
create index ex_s_last on ex_server\
(\
   last\
);\
"
std::map<int, const char*> FS_VER_SQL() {
    std::map<int, const char*> ver;
    ver[0] = FS_SQL_V0;
    return ver;
}
std::map<int, const char*> EX_VER_SQL() {
    std::map<int, const char*> ver;
    ver[0] = TS_SQL_V0;
    return ver;
}
}
}
