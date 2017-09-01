

#ifndef __profileutils_h__
#define __profileutils_h__

#ifdef __cplusplus
extern "C"
{
#endif
          

#include <stdint.h>
#include <time.h>

#define SEQ_MSG_LEN 6
#define SEQ_NAME_LEN 2

/*typedef struct
{
   char data[SEQ_MSG_LEN];
} seqmsg;
*/
typedef struct
{
   char seqmsg[SEQ_MSG_LEN];
   char seqpt[4];
   struct timespec seqtm;
} seqlog;

typedef struct
{
   char app[2];
   seqlog sl;
} sequence_report;

int parse_seqmsg(char * app, uint32_t *seqnum, const char *sm);

int load_seqmsg(char *sm, const char * app, uint32_t seqnum);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //__profileutils_h__
