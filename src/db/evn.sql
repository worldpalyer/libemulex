/*==============================================================*/
/* Table: _env                                                */
/*==============================================================*/
create table _env_
(
   name                 varchar(32) not null,
   value                varchar(10240),
   primary key (name)
);