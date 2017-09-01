

#include "profileutils.h"


int parse_seqmsg(char *app, uint32_t *seqnum, const char * sm)
{
   char *sn;
   
   app[0] = sm[0];
   app[1] = sm[1];
   
   sn = (char*) seqnum;
   
   sn[0] = sm[0 + SEQ_NAME_LEN];
   sn[1] = sm[1 + SEQ_NAME_LEN];
   sn[2] = sm[2 + SEQ_NAME_LEN];
   sn[3] = sm[3 + SEQ_NAME_LEN];
   
   return 0;
}


int load_seqmsg(char * sm, const char * app, uint32_t seqnum)
{
   char *sn;
   
   sm[0] = app[0];
   sm[1] = app[1];
   
   sn = (char*) &seqnum;
   
   sm[0 + SEQ_NAME_LEN] = sn[0];
   sm[1 + SEQ_NAME_LEN] = sn[1];
   sm[2 + SEQ_NAME_LEN] = sn[2];
   sm[3 + SEQ_NAME_LEN] = sn[3];
   
   return 0;
}

