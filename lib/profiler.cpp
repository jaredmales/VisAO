
#include "profiler.h"

namespace VisAO
{

profiler::profiler(int nlogs)
{
   logs = 0;
  
   set_numlogs(nlogs);
   
   fd = 0;
   logs_per_file = 10000;
   
   pthread_mutex_init(&my_mutex, NULL);
   
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
   
   pthread_mutex_init(&thmutex, &attr);
   
   
   pthread_cond_init(&thcond, NULL);
   
}

int profiler::set_numlogs(int nlogs)
{
  if(logs) delete[] logs;
  
  numlogs = nlogs;
  logs = new seqlog[numlogs];
  currlog = -1;
  abscurrlog = -1;
  abslast_written = -1;
  last_written = 0;
  
  return 0;
}

int profiler::set_base_path(const std::string& bp)
{
   base_path = bp;
   return 0;
}
      
int profiler::get_nextlog(int clog)
{
   if(clog == numlogs - 1) return 0;
   else return clog+1;
}
      
int profiler::logseqmsg(const char * sm, const char * sp, timespec *st)
{
   //if(pthread_mutex_trylock(&my_mutex) < 0) return -1;
    
   currlog = get_nextlog(currlog);
   abscurrlog++;
   
   memcpy(&(logs[currlog].seqmsg), sm, SEQ_MSG_LEN);
   memcpy(logs[currlog].seqpt, sp, 4);
   logs[currlog].seqtm.tv_sec = st->tv_sec;
   logs[currlog].seqtm.tv_nsec = st->tv_nsec;
 
   //pthread_mutex_unlock(&my_mutex);
   
   return 0;
}
   
bool profiler::logs_to_write()
{
   if(abslast_written < abscurrlog) return true;
   else return false;
}

int profiler::write_logs()
{
   while(abslast_written < abscurrlog && fd >= 0)
   {
      if(fd == 0)
      {
         get_visao_filename(fnamebuff, &(logs[last_written].seqtm) );
         curr_path = base_path + "_" + fnamebuff + ".prof";
         fd = open(curr_path.c_str(), O_WRONLY | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
         
         if(fd <= 0)
         {
            std::cerr << "Profiler error opening file: " << curr_path << "\n";
            fd = 0;
            return -1;
         }
         
         logs_in_file =0;
      }
                                                  
      /********** For Testing *****************
      char app[3];
      char spt[5];
      uint32_t sn;
      double st;
   
      parse_seqmsg(app, &sn, (logs[last_written].seqmsg));
      app[2] = '\0';
      memcpy(spt, logs[last_written].seqpt, 4);
      spt[4] = '\0';
      
      st = ((double)logs[last_written].seqtm.tv_sec+((double)logs[last_written].seqtm.tv_nsec/1e9));
   
      //std::cout.precision(20);
      //std::cout << "Profiler: " << last_written << " " << app << " " << sn << " " << spt << " " << st  << "\n";
      */
      /***************************************/
      
      write(fd, &(logs[last_written]), sizeof(seqlog));
      logs_in_file++;
      last_written = get_nextlog(last_written);
      abslast_written++;
      
      if(logs_in_file >= logs_per_file)
      {
         close(fd);
         fd = 0;
      }     
   }
   return 0;
}


void profiler::start()
{
   //signal(SIGIO, SIG_IGN);

   sigset_t set;
   sigemptyset(&set);
   sigaddset(&set, SIGIO);

   pthread_sigmask(SIG_BLOCK, &set, 0);

   std::cout << "profiler started\n";
   //LOG_INFO("profiler started\n");
   while(1)
   {
      pthread_mutex_lock(&thmutex);
      pthread_cond_wait(&thcond, &thmutex);
      pthread_mutex_unlock(&thmutex);
      write_logs();
   }
   return;
}


}//namespace VisAO

