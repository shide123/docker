#ifndef __OPTABLE_H
#define __OPTABLE_H
#include <db/sink.h>
#include <db/table.h>
#include <com/slist.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
struct tbgroupinfomation* clone_tbgroupinfomation(struct tbgroupinfomation *);
int op_tbgroupinfomation(Sink         *sink,
                   struct tbgroupinfomation *ts,
                   int            op,
                   const char    *filter);
int get_from_tbgroupinfomation (Sink *sink, SList **list, const char *filter);
struct tbgroupinfomation * safe_malloc_tbgroupinfomation();
void safe_free_tbgroupinfomation(struct tbgroupinfomation *rec);
struct tbsystemmsg* clone_tbsystemmsg(struct tbsystemmsg *);
int op_tbsystemmsg(Sink         *sink,
                   struct tbsystemmsg *ts,
                   int            op,
                   const char    *filter);
int get_from_tbsystemmsg (Sink *sink, SList **list, const char *filter);
struct tbsystemmsg * safe_malloc_tbsystemmsg();
void safe_free_tbsystemmsg(struct tbsystemmsg *rec);
struct dks_useraccount* clone_tbuseraccount(struct dks_useraccount *);
int op_tbuseraccount(Sink         *sink,
                   struct dks_useraccount *ts,
                   int            op,
                   const char    *filter);
int get_from_tbuseraccount (Sink *sink, SList **list, const char *filter);
struct dks_useraccount * safe_malloc_tbuseraccount();
void safe_free_tbuseraccount(struct dks_useraccount *rec);
struct tbusercollroom* clone_tbusercollroom(struct tbusercollroom *);
int op_tbusercollroom(Sink         *sink,
                   struct tbusercollroom *ts,
                   int            op,
                   const char    *filter);
int get_from_tbusercollroom (Sink *sink, SList **list, const char *filter);
struct tbusercollroom * safe_malloc_tbusercollroom();
void safe_free_tbusercollroom(struct tbusercollroom *rec);
struct dks_user* clone_tbuserinformation(struct dks_user *);
int op_tbuserinformation(Sink         *sink,
                   struct dks_user *ts,
                   int            op,
                   const char    *filter);
int get_from_tbuserinformation (Sink *sink, SList **list, const char *filter);
struct dks_user * safe_malloc_tbuserinformation();
void safe_free_tbuserinformation(struct dks_user *rec);
struct tbuserlog* clone_tbuserlog(struct tbuserlog *);
int op_tbuserlog(Sink         *sink,
                   struct tbuserlog *ts,
                   int            op,
                   const char    *filter);
int get_from_tbuserlog (Sink *sink, SList **list, const char *filter);
struct tbuserlog * safe_malloc_tbuserlog();
void safe_free_tbuserlog(struct tbuserlog *rec);
struct tbusermsg* clone_tbusermsg(struct tbusermsg *);
int op_tbusermsg(Sink         *sink,
                   struct tbusermsg *ts,
                   int            op,
                   const char    *filter);
int get_from_tbusermsg (Sink *sink, SList **list, const char *filter);
struct tbusermsg * safe_malloc_tbusermsg();
void safe_free_tbusermsg(struct tbusermsg *rec);
struct tbuserstatus* clone_tbuserstatus(struct tbuserstatus *);
int op_tbuserstatus(Sink         *sink,
                   struct tbuserstatus *ts,
                   int            op,
                   const char    *filter);
int get_from_tbuserstatus (Sink *sink, SList **list, const char *filter);
struct tbuserstatus * safe_malloc_tbuserstatus();
void safe_free_tbuserstatus(struct tbuserstatus *rec);

//*-- 表结构修改了，不再使用2007-12-23 ----------------*/
struct tbusertradelog* clone_tbusertradelog(struct tbusertradelog *);
int op_tbusertradelog(Sink         *sink,
                   struct tbusertradelog *ts,
                   int            op,
                   const char    *filter);
int get_from_tbusertradelog (Sink *sink, SList **list, const char *filter);
struct tbusertradelog * safe_malloc_tbusertradelog();
void safe_free_tbusertradelog(struct tbusertradelog *rec);
//*-- END -----------------*/

struct dks_vcbinformation* clone_tbvcbinformation(struct dks_vcbinformation *);
int op_tbvcbinformation(Sink         *sink,
                   struct dks_vcbinformation *ts,
                   int            op,
                   const char    *filter);
int get_from_tbvcbinformation (Sink *sink, SList **list, const char *filter);
struct dks_vcbinformation * safe_malloc_tbvcbinformation();
void safe_free_tbvcbinformation(struct dks_vcbinformation *rec);
struct tbvcbmember* clone_tbvcbmember(struct tbvcbmember *);
int op_tbvcbmember(Sink         *sink,
                   struct tbvcbmember *ts,
                   int            op,
                   const char    *filter);
int get_from_tbvcbmember (Sink *sink, SList **list, const char *filter);
struct tbvcbmember * safe_malloc_tbvcbmember();
void safe_free_tbvcbmember(struct tbvcbmember *rec);
struct tbvcbprice* clone_tbvcbprice(struct tbvcbprice *);
int op_tbvcbprice(Sink         *sink,
                   struct tbvcbprice *ts,
                   int            op,
                   const char    *filter);
int get_from_tbvcbprice (Sink *sink, SList **list, const char *filter);
struct tbvcbprice * safe_malloc_tbvcbprice();
void safe_free_tbvcbprice(struct tbvcbprice *rec);
struct dks_vcbstatus* clone_tbvcbstatus(struct dks_vcbstatus *);
int op_tbvcbstatus(Sink         *sink,
                   struct dks_vcbstatus *ts,
                   int            op,
                   const char    *filter);
int get_from_tbvcbstatus (Sink *sink, SList **list, const char *filter);
struct dks_vcbstatus * safe_malloc_tbvcbstatus();
void safe_free_tbvcbstatus(struct dks_vcbstatus *rec);

struct tbvcbstoreinfo* clone_tbvcbstoreinfo(struct tbvcbstoreinfo *);
int op_tbvcbstoreinfo(Sink         *sink,
                   struct tbvcbstoreinfo *ts,
                   int            op,
                   const char    *filter);
int get_from_tbvcbstoreinfo (Sink *sink, SList **list, const char *filter);
struct tbvcbstoreinfo * safe_malloc_tbvcbstoreinfo();
void safe_free_tbvcbstoreinfo(struct tbvcbstoreinfo *rec);

struct tbvcboptionlog* clone_tbvcboptionlog(struct tbvcboptionlog *);
int op_tbvcboptionlog(Sink        *sink,
                      struct tbvcboptionlog *tp,
	                  int           op,
	                  const char    *filter);
int get_from_tbvcboptionlog (Sink *sink, SList **list, const char *filter);
struct tbvcboptionlog* safe_malloc_tbvcboptionlog();
void safe_free_tbvcboptionlog(struct tbvcboptionlog *rec);

struct tbusergiftinfo* clone_tbusergiftinfo(struct tbusergiftinfo *);
int op_tbusergiftinfo(Sink         *sink,
                      struct tbusergiftinfo *ts,
	                  int            op,
	                  const char    *filter);
int get_from_tbusergiftinfo (Sink *sink, SList **list, const char *filter);
struct tbusergiftinfo * safe_malloc_tbusergiftinfo();
void safe_free_tbusergiftinfo(struct tbusergiftinfo *rec);
//===========================new add==========================
int op_tbuserbuddys(Sink         *sink,
                    struct tbuserbuddys *ts,
	                int            op,
	                const char    *filter);


#ifdef __cplusplus
}
#endif
#endif
