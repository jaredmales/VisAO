

#ifndef __profiler_h__
#define __profiler_h__

#include <string>
#include <cstring>
#include <iostream>


#include <pthread.h>
#include <signal.h>

#include "profileutils.h"

#include "visaoimutils.h" // for get_visao_filename

///The VisAO namespace
namespace VisAO
{

class profiler
{
   public:
      profiler(int nlogs);

      ~profiler();
      
      pthread_mutex_t thmutex;
      pthread_cond_t thcond;

   protected:
      int numlogs;
      int currlog;
      int abscurrlog;      
     
      seqlog * logs;

      std::string base_path;
      char fnamebuff[22];
      std::string curr_path;
      
      int fd; ///< file descriptor of the currently open file.
      
      int logs_per_file; ///< maximum number of logs to write per file.
      int logs_in_file; ///< number of logs in current file.
      
      int last_written; ///< index of the last file written.
      int abslast_written; ///< last file written
      
      pthread_mutex_t my_mutex;
      
   public:
      int set_numlogs(int nlogs);
      int get_numlogs(){return numlogs;}
      
      int get_currlog(){return currlog;}
      int get_abscurrlog(){return abscurrlog;}
      
      seqlog * get_seqlog(){return logs;}
      
    
      int set_base_path(const std::string& bp);
      std::string get_base_path(){return base_path;}
      
      int get_nextlog(int clog);
      
      int logseqmsg(const char * sm, const char * sp, timespec *st);

      bool logs_to_write();

      int write_logs();

      void start();
      
};// class profiler

}//namespace VisAO

#endif //__profiler_h__


